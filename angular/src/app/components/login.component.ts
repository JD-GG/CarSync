import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CommonModule } from '@angular/common';
import { AuthService } from '../../services/auth.service';

@Component({
  selector: 'app-login',
  standalone: true,
  imports: [FormsModule, CommonModule],
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.css']
})
export class LoginComponent {
  username = '';
  password = '';
  message = '';

  constructor(private auth: AuthService) {}

  onLogin() {
    this.auth.login(this.username, this.password).subscribe({
      next: () => this.message = 'Login erfolgreich!',
      error: err => this.message = err.error?.error || 'Login fehlgeschlagen'
    });
  }
}
