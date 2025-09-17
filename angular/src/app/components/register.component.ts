import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CommonModule } from '@angular/common';
import { AuthService } from '../../services/auth.service';
import { NgxMaskDirective } from 'ngx-mask';

@Component({
  selector: 'app-register',
  standalone: true,
  imports: [FormsModule, CommonModule, NgxMaskDirective],
  templateUrl: './register.component.html'
})

export class RegisterComponent {
  username = '';
  password = '';
  message = '';
  mac = '';

  constructor(private auth: AuthService) {}

  onRegister() {
    this.auth.register(this.username, this.password, this.mac).subscribe({
      next: () => this.message = 'Registrierung erfolgreich!',
      error: err => this.message = err.error?.error || 'Registrierung fehlgeschlagen'
    });
  }
}
