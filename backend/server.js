import express from "express";
import mysql from "mysql2/promise";
import bcrypt from "bcrypt";
import bodyParser from "body-parser";
import dotenv from "dotenv";
import jwt from "jsonwebtoken";
import { InfluxDB } from "@influxdata/influxdb-client";

// Load environment variables
dotenv.config();
const host = process.env.MYSQL_HOST;
const port = process.env.MYSQL_PORT;
const database = process.env.MYSQL_DATABASE;
const user = process.env.MYSQL_USER;
const pw = process.env.MYSQL_PASSWORD;
const jwtSecret = process.env.JWT_SECRET;
const influxUrl = process.env.INFLUX_URL;
const influxToken = process.env.INFLUX_TOKEN;
const influxOrg = process.env.INFLUX_ORG || process.env.INFLUXDB_ORG;
const influxBucket = process.env.INFLUX_BUCKET || process.env.INFLUXDB_BUCKET;

if (!jwtSecret) {
  throw new Error("JWT_SECRET environment variable is required");
}

if (!influxToken) {
  throw new Error("INFLUX_TOKEN environment variable is required");
}

if (!influxOrg) {
  throw new Error("INFLUX_ORG environment variable is required");
}

if (!influxBucket) {
  throw new Error("INFLUX_BUCKET environment variable is required");
}

const app = express();
app.use(bodyParser.json());

const pool = mysql.createPool({
  port: port,
  host: host,
  user: user,
  password: pw,
  database: database
});

const influxQueryApi = new InfluxDB({ url: influxUrl, token: influxToken }).getQueryApi(influxOrg);

const containsForbiddenCharacter = (value) => typeof value === "string" && value.includes(";");
const hasUnsafeInput = (...values) => values.some(containsForbiddenCharacter);

const authenticateToken = (req, res, next) => {
  const authHeader = req.headers["authorization"]; // Express normalizes header casing
  const token = authHeader?.split(" ")[1];

  if (!token) {
    return res.status(401).json({ error: "Missing authorization token" });
  }

  jwt.verify(token, jwtSecret, (err, payload) => {
    if (err || !payload?.id) {
      return res.status(401).json({ error: "Invalid or expired token" });
    }

    req.user = payload;
    next();
  });
};

const isValidRange = (value) => {
  if (typeof value !== "string") {
    return false;
  }
  return /^-[0-9]+(m|h|d|w)$/.test(value.trim());
};

// Registrierung
app.post("/register", async (req, res) => {
  const { username, password, mac } = req.body;
  if (!username || !password || !mac) return res.status(400).json({ error: "Missing data" });
  if (hasUnsafeInput(username, password, mac)) return res.status(400).json({ error: "Invalid characters in input" });

  const hash = await bcrypt.hash(password, 10);
  const macPrefix = "0x";
  const macHex = macPrefix + mac;
  const macInt = parseInt(macHex, 16);
  

  try {
    await pool.query("INSERT INTO benutzer (username, password, macInt) VALUES (?, ?, ?)", [username, hash, macInt]);
    res.json({ message: "User created" });
  } catch (err) {
    if (err.code === "ER_DUP_ENTRY") return res.status(400).json({ error: "Username already exists" });
    res.status(500).json({ error: "DB error" });
  }
});

// Login
app.post("/login", async (req, res) => {
  const { username, password } = req.body;
  if (!username || !password) return res.status(400).json({ error: "Missing data" });
  if (hasUnsafeInput(username, password)) return res.status(400).json({ error: "Invalid characters in input" });
  const [rows] = await pool.query("SELECT * FROM benutzer WHERE username=?", [username]);

  if (rows.length === 0) return res.status(400).json({ error: "Invalid credentials" });

  const valid = await bcrypt.compare(password, rows[0].password);
  if (!valid) return res.status(400).json({ error: "Invalid credentials" });

  const token = jwt.sign({ id: rows[0].id, username: rows[0].username }, jwtSecret, { expiresIn: "1h" });

  res.json({ message: "Login successful", token });
});

app.get("/rpm-data", authenticateToken, async (req, res) => {
  try {
    const [rows] = await pool.query("SELECT macInt FROM benutzer WHERE id = ?", [req.user.id]);

    if (!rows.length) {
      return res.status(404).json({ error: "User not found" });
    }

    const macIntRaw = rows[0].macInt;
    const macInt = Number(macIntRaw);

    if (!Number.isFinite(macInt)) {
      return res.status(500).json({ error: "Invalid MAC stored for user" });
    }

    const pointLimit = 200;
    const fluxQuery = `
      rpm = from(bucket: "${influxBucket}")
        |> range(start: -3d)
        |> filter(fn: (r) => r._measurement == "data" and r._field == "rpm")
        |> rename(columns: {_value: "rpm"})
        |> keep(columns: ["_time", "rpm"])

      mac = from(bucket: "${influxBucket}")
        |> range(start: -3d)
        |> filter(fn: (r) => r._measurement == "data" and r._field == "mac")
        |> rename(columns: {_value: "mac"})
        |> keep(columns: ["_time", "mac"])

      join(
        tables: {rpm: rpm, mac: mac},
        on: ["_time"]
      )
      |> filter(fn: (r) => r.mac == ${macInt})
      |> keep(columns: ["_time", "rpm"])
      |> sort(columns: ["_time"])
    `;

    const points = [];
    try {
      for await (const { values, tableMeta } of influxQueryApi.iterateRows(fluxQuery)) {
        const row = tableMeta.toObject(values);
        const rpm = Number(row.rpm);
        if (!Number.isFinite(rpm)) {
          continue;
        }
        points.push({ time: row._time, rpm });
      }
    } catch (err) {
      console.error("Failed to query InfluxDB:", err);
      return res.status(500).json({ error: "Failed to query telemetry data" });
    }

    // If no points, generate and write 5 fake points (1 per minute for last 5 minutes)
    if (points.length === 0) {
      const { Point, InfluxDB } = await import("@influxdata/influxdb-client");
      const influxWriteApi = new InfluxDB({ url: influxUrl, token: influxToken }).getWriteApi(influxOrg, influxBucket, "ms");
      const now = Date.now();
      const fakePoints = [];
      for (let i = 5; i > 0; i--) {
        const timestamp = now - i * 60 * 1000; // i minutes ago
        const rpm = Math.floor(Math.random() * (3000 - 800 + 1)) + 800;
        const point = new Point("data")
          .intField("rpm", rpm)
          .intField("mac", macInt)
          .timestamp(new Date(timestamp));
        fakePoints.push({ time: new Date(timestamp).toISOString(), rpm });
        influxWriteApi.writePoint(point);
      }
      await influxWriteApi.close();
      return res.json({ points: fakePoints});
    }

    res.json({ points});
  } catch (err) {
    console.error("Failed to fetch RPM data:", err);
    res.status(500).json({ error: "Failed to fetch RPM data" });
  }
});

// Simple GET endpoint for testing
app.get('/ping', (req, res) => {
  res.json({ message: 'pong' });
});

app.listen(3000, () => console.log("API running on port 3000"));

