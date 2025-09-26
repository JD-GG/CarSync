import { inject } from '@angular/core';
import { CanActivateFn, Router } from '@angular/router';
import { AuthService } from '../../services/auth.service';

// Guard that redirects guests to the login view before opening protected routes.

export const authGuard: CanActivateFn = () => {
  const auth = inject(AuthService);
  // Allow access immediately when the current session is valid.
  if (auth.isAuthenticated()) {
    return true;
  }

  const router = inject(Router);
  // Redirect guests toward the login screen.
  return router.createUrlTree(['/login']);
};
