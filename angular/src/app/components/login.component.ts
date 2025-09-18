import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CommonModule } from '@angular/common';
import { Router } from '@angular/router';
import { RouterModule } from '@angular/router';
import { AuthService } from '../../services/auth.service';

@Component({
  selector: 'app-login',
  standalone: true,
  imports: [FormsModule, CommonModule, RouterModule],
  templateUrl: './login.component.html'
})
export class LoginComponent {
  username = '';
  password = '';
  message = '';

  constructor(private auth: AuthService, private router: Router) {}

  onLogin() {
    this.auth.login(this.username, this.password).subscribe({
      next: (response: any) => {
        const token = response?.token;
        if (token) {
          this.auth.setSession(token, this.username);
        }
        this.message = response?.message || 'Login erfolgreich!';
        this.router.navigate(['/dashboard']);
      },
      error: err => {
        this.message = err.error?.error || 'Login fehlgeschlagen';
      }
    });
  }
}
