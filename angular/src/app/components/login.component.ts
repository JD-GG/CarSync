import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CommonModule } from '@angular/common';
import { Router } from '@angular/router';
import { RouterModule } from '@angular/router';
import { AuthService } from '../../services/auth.service';

// Login screen that delegates authentication to the shared AuthService.

@Component({
  selector: 'app-login',
  standalone: true,
  imports: [FormsModule, CommonModule, RouterModule],
  templateUrl: './login.component.html'
})
export class LoginComponent {
  // Form-bound credential fields and feedback message.
  username = '';
  password = '';
  message = '';

  constructor(private auth: AuthService, private router: Router) {}

  // Submit entered credentials and route the user to the dashboard on success.
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
