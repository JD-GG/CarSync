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
  mac = '';

  formatMac(event: any) {
  let value = event.target.value.replace(/[^A-Fa-f0-9]/g, ''); // nur Hex
  let parts = value.match(/.{1,2}/g) || [];                    // in 2er-Gruppen teilen
  this.mac = parts.join('').substr(0, 17);
  }
  constructor(private auth: AuthService) {}

  onRegister() {
    this.auth.register(this.username, this.password, this.mac).subscribe({
      next: () => this.message = 'Registrierung erfolgreich!',
      error: err => this.message = err.error?.error || 'Registrierung fehlgeschlagen'
    });
  }
}
