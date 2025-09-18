import { Component, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
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

  readonly chartWidth = 1000;
  readonly chartHeight = 320;
  readonly maxRpm = 7000;

  readonly rpmData = [1200, 2800, 4300, 5200, 6100, 5400, 4800];
  readonly timeLabels = ['-30m', '-25m', '-20m', '-15m', '-10m', '-5m', 'Jetzt'];
  readonly yTicks = Array.from({ length: 8 }, (_, i) => i * 1000);

  chartPath = '';
  chartAreaPath = '';

  constructor(private auth: AuthService) {}

  ngOnInit(): void {
    this.username = this.auth.getUsername() || 'Driver';
    this.computePaths();
  }

  get viewBox(): string {
    return `0 0 ${this.chartWidth} ${this.chartHeight}`;
  }

  private computePaths(): void {
    if (!this.rpmData.length) {
      this.chartPath = '';
      this.chartAreaPath = '';
      return;
    }

    const segmentWidth = this.chartWidth / Math.max(1, this.rpmData.length - 1);
    const points = this.rpmData.map((value, index) => {
      const x = index * segmentWidth;
      const y = this.valueToY(value);
      return { x, y };
    });

    this.chartPath = points
      .map((point, index) => `${index === 0 ? 'M' : 'L'}${point.x} ${point.y}`)
      .join(' ');

    const areaPoints = [
      `M0 ${this.chartHeight}`,
      ...points.map(point => `L${point.x} ${point.y}`),
      `L${(this.rpmData.length - 1) * segmentWidth} ${this.chartHeight}`,
      'Z'
    ];

    this.chartAreaPath = areaPoints.join(' ');
  }

  valueToY(value: number): number {
    const clamped = Math.max(0, Math.min(this.maxRpm, value));
    const ratio = clamped / this.maxRpm;
    return this.chartHeight - ratio * this.chartHeight;
  }
}
