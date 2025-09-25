import { Component, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { HttpClient, HttpHeaders } from '@angular/common/http';
import { finalize } from 'rxjs';
import { AuthService } from '../../services/auth.service';

@Component({
  selector: 'app-dashboard',
  standalone: true,
  imports: [CommonModule],
  templateUrl: './dashboard.component.html',
  styles: [`
    .dashboard-card {
      max-width: 1100px;
      margin: 0 auto;
    }

    .chart-wrapper {
      position: relative;
      width: 100%;
      min-height: 320px;
    }

    .chart-bg {
      fill: rgba(255, 255, 255, 0.03);
    }

    .chart-grid-line {
      stroke: rgba(255, 255, 255, 0.08);
      stroke-width: 1;
    }

    .chart-axis {
      stroke: rgba(255, 255, 255, 0.35);
      stroke-width: 1.5;
    }

    .chart-line {
      fill: none;
      stroke: #4dabff;
      stroke-width: 3;
      stroke-linecap: round;
      stroke-linejoin: round;
    }

    .chart-shadow {
      fill: url(#rpmGradient);
      opacity: 0.35;
    }

    .tick-label {
      font-size: 0.7rem;
      fill: rgba(248, 249, 250, 0.65);
    }
  `]
})
export class DashboardComponent implements OnInit {
  username = '';

  readonly apiUrl = '/api';
  readonly chartWidth = 1000;
  readonly chartHeight = 320;
  readonly maxRpm = 7000;
  readonly yTicks = Array.from({ length: 8 }, (_, i) => i * 1000);

  rpmData: number[] = [];
  timeLabels: string[] = [];
  chartPath = '';
  chartAreaPath = '';
  segmentWidth = this.chartWidth;

  isLoading = false;
  errorMessage = '';

  constructor(private auth: AuthService, private http: HttpClient) {}

  ngOnInit(): void {
    this.username = this.auth.getUsername() || 'Driver';
    this.fetchRpmData();
  }

  get viewBox(): string {
    return `0 0 ${this.chartWidth} ${this.chartHeight}`;
  }

  get hasData(): boolean {
    return this.rpmData.length > 0;
  }

  get latestRpm(): number | null {
    return this.hasData ? this.rpmData[this.rpmData.length - 1] : null;
  }

  valueToY(value: number): number {
    const clamped = Math.max(0, Math.min(this.maxRpm, value));
    const ratio = clamped / this.maxRpm;
    return this.chartHeight - ratio * this.chartHeight;
  }

  private fetchRpmData(): void {
    const token = this.auth.getToken();

    if (!token) {
      this.errorMessage = 'Nicht angemeldet.';
      this.computePaths();
      return;
    }

    const headers = new HttpHeaders({ Authorization: `Bearer ${token}` });

    this.isLoading = true;
    this.errorMessage = '';

    this.http.get<RpmDataResponse>(`${this.apiUrl}/rpm-data`, { headers })
      .pipe(finalize(() => (this.isLoading = false)))
      .subscribe({
        next: response => {
          const points = Array.isArray(response?.points) ? response.points : [];

          this.rpmData = points.map(point => point.rpm);
          this.timeLabels = points.map(point => this.formatUtcLabel(point.time));

          if (!this.rpmData.length) {
            this.errorMessage = 'Keine RPM-Daten für deine MAC gefunden.';
          }

          this.computePaths();
        },
        error: err => {
          this.errorMessage = err?.error?.error || 'Fehler beim Laden der RPM-Daten.';
          this.rpmData = [];
          this.timeLabels = [];
          this.computePaths();
        }
      });
  }

  private formatUtcLabel(timestamp: string): string {
    const date = new Date(timestamp);
    if (Number.isNaN(date.getTime())) {
      return '';
    }

    return new Intl.DateTimeFormat('de-DE', {
      hour: '2-digit',
      minute: '2-digit',
      hourCycle: 'h23',
      timeZone: 'UTC'
    }).format(date);
  }

  private computePaths(): void {
    if (!this.rpmData.length) {
      this.chartPath = '';
      this.chartAreaPath = '';
      this.segmentWidth = this.chartWidth;
      return;
    }

    this.segmentWidth = this.rpmData.length > 1
      ? this.chartWidth / (this.rpmData.length - 1)
      : this.chartWidth;

    const points = this.rpmData.map((value, index) => {
      const x = index * this.segmentWidth;
      const y = this.valueToY(value);
      return { x, y };
    });

    this.chartPath = points
      .map((point, index) => `${index === 0 ? 'M' : 'L'}${point.x} ${point.y}`)
      .join(' ');

    const areaPoints = [
      `M0 ${this.chartHeight}`,
      ...points.map(point => `L${point.x} ${point.y}`),
      `L${(this.rpmData.length - 1) * this.segmentWidth} ${this.chartHeight}`,
      'Z'
    ];

    this.chartAreaPath = areaPoints.join(' ');
  }
}

interface RpmPoint {
  time: string;
  rpm: number;
}

interface RpmDataResponse {
  points: RpmPoint[];
}
