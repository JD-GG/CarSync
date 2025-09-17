import express from "express";
import mysql from "mysql2/promise";
import bcrypt from "bcrypt";
import bodyParser from "body-parser";
import dotenv from "dotenv";

// Load environment variables
dotenv.config();
const port = process.env.MARIADB_PORT;
const user = process.env.MARIADB_USER;
const pw = process.env.MARIADB_PASSWORD;
const database = process.env.MARIADB_DATABASE;

const app = express();
app.use(bodyParser.json());

const pool = mysql.createPool({
  host: "mariadb",
  port: port,
  user: user,
  password: pw,
  database: database
});

// Registrierung
app.post("/register", async (req, res) => {
  const { username, password, mac } = req.body;
  if (!username || !password || !mac) return res.status(400).json({ error: "Missing data" });

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
  const [rows] = await pool.query("SELECT * FROM benutzer WHERE username=?", [username]);

  if (rows.length === 0) return res.status(400).json({ error: "Invalid credentials" });

  const valid = await bcrypt.compare(password, rows[0].password);
  if (!valid) return res.status(400).json({ error: "Invalid credentials" });

  res.json({ message: "Login successful" });
});

// Simple GET endpoint for testing
app.get('/ping', (req, res) => {
  res.json({ message: 'pong' });
});

// Create table if it doesn't exist
async function ensureTable() {
  await pool.query(`
    CREATE TABLE IF NOT EXISTS benutzer (
      id INT AUTO_INCREMENT PRIMARY KEY,
      username VARCHAR(255) NOT NULL UNIQUE,
      password VARCHAR(255) NOT NULL
    )
  `);
}

ensureTable().then(() => {
  app.listen(3000, () => console.log("API running on port 3000"));
}).catch(err => {
  console.error("Failed to ensure table:", err);
  process.exit(1);
});
