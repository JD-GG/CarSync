import { bootstrapApplication } from '@angular/platform-browser';
import { AppComponent } from './app/app.component';

// Startet die App direkt mit der Standalone-Komponente
bootstrapApplication(AppComponent)
  .catch(err => console.error(err));
