import { bootstrapApplication } from '@angular/platform-browser';
import { provideRouter, Routes } from '@angular/router';
import { provideHttpClient } from '@angular/common/http';
import { importProvidersFrom } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { provideNgxMask } from 'ngx-mask';

import { AppComponent } from './app/app.component';
import { LoginComponent } from './app/components/login.component';
import { RegisterComponent } from './app/components/register.component';

const routes: Routes = [
  { path: 'login', component: LoginComponent },
  { path: 'register', component: RegisterComponent },
  { path: '', redirectTo: 'login', pathMatch: 'full' }
];

bootstrapApplication(AppComponent, {
  providers: [
    provideHttpClient(),
    importProvidersFrom(FormsModule),
    provideRouter(routes),
    provideNgxMask({
      patterns: {
        'X': { pattern: /[0-9A-Fa-f]/ }
      }
    })
  ]
}).catch(err => console.error(err));
