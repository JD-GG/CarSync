import { Component } from '@angular/core';
import { RouterModule } from '@angular/router';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [RouterModule],
  template: `
    <h1>Benutzerverwaltung</h1>
    <nav>
      <a routerLink="/login" routerLinkActive="active">Login</a> |
      <a routerLink="/register" routerLinkActive="active">Register</a>
    </nav>
    <hr>
    <router-outlet></router-outlet>
  `,
  styles: [`
    nav a.active { font-weight: bold; }
  `]
})
export class AppComponent {}
