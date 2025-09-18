import { Component } from '@angular/core';
import { RouterModule } from '@angular/router';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [RouterModule],
  template: `
    <div class="main-shell d-flex flex-column text-light">
      <header class="py-4">
        <div class="container px-4 px-md-5">
          <nav class="navbar navbar-expand-md navbar-dark glass-card rounded-4 px-4 py-3">
            <a class="navbar-brand fw-semibold" routerLink="/">CarSync</a>
            <div class="ms-auto d-flex gap-2">
              <a class="btn btn-outline-light" routerLink="/login" routerLinkActive="active-link" [routerLinkActiveOptions]="{ exact: true }">Login</a>
              <a class="btn btn-primary" routerLink="/register" routerLinkActive="active-link">Register</a>
            </div>
          </nav>
        </div>
      </header>

      <main class="flex-fill d-flex align-items-center justify-content-center px-3 pb-5">
        <router-outlet></router-outlet>
      </main>

      <footer class="py-4 text-center text-white-50 small">
        Crafted with ❤️ for smooth fleet syncing
      </footer>
    </div>
  `,
  styles: [`
    :host { display: block; height: 100%; }
    .active-link {
      background-color: rgba(13, 110, 253, 0.2);
      border-color: rgba(13, 110, 253, 0.35);
    }
    footer {
      letter-spacing: 0.05em;
    }
  `]
})
export class AppComponent {}
