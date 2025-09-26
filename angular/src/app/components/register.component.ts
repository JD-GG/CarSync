import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CommonModule } from '@angular/common';
import { RouterModule } from '@angular/router';
import { AuthService } from '../../services/auth.service';
import { NgxMaskDirective } from 'ngx-mask';


// Registration screen that captures credentials plus a MAC address.
@Component({
  selector: 'app-register',
  standalone: true,
  imports: [FormsModule, CommonModule, RouterModule, NgxMaskDirective],
  templateUrl: './register.component.html'
})

export class RegisterComponent {
  // Bound form fields for credentials, MAC entry, and API feedback.
  username = '';
  password = '';
  message = '';
  mac = '';

  // AuthService encapsulates the HTTP calls for account creation.
  constructor(private auth: AuthService) {}

  // Forward the form data to the API and surface any feedback to the user.
  onRegister() {
    this.auth.register(this.username, this.password, this.mac).subscribe({
      next: () => this.message = 'Registrierung erfolgreich!',
      error: err => this.message = err.error?.error || 'Registrierung fehlgeschlagen'
    });
  }
}
