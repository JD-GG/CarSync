import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CommonModule } from '@angular/common';
import { AuthService } from '../../services/auth.service';

@Component({
  selector: 'app-register',
  standalone: true,
  imports: [FormsModule, CommonModule],
  templateUrl: './register.component.html'
})
export class RegisterComponent {
  username = '';
  password = '';
  message = '';

  constructor(private auth: AuthService) {}

  onRegister() {
    this.auth.register(this.username, this.password).subscribe({
      next: () => this.message = 'Registrierung erfolgreich!',
      error: err => this.message = err.error?.error || 'Registrierung fehlgeschlagen'
    });
  }
}
