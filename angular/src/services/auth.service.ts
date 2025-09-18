import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';

@Injectable({ providedIn: 'root' })
export class AuthService {
  private apiUrl = '/api';  
  private readonly tokenKey = 'carSyncToken';
  private readonly userKey = 'carSyncUser';

  constructor(private http: HttpClient) {}

  login(username: string, password: string) {
    return this.http.post(`${this.apiUrl}/login`, { username, password });
  }

  register(username: string, password: string, mac: string) {
    return this.http.post(`${this.apiUrl}/register`, { username, password, mac });
  }

  setSession(token: string, username: string) {
    const storage = this.getStorage();
    if (!storage) return;
    storage.setItem(this.tokenKey, token);
    storage.setItem(this.userKey, username);
  }

  clearSession() {
    const storage = this.getStorage();
    if (!storage) return;
    storage.removeItem(this.tokenKey);
    storage.removeItem(this.userKey);
  }

  getToken(): string | null {
    const storage = this.getStorage();
    return storage ? storage.getItem(this.tokenKey) : null;
  }

  getUsername(): string | null {
    const storage = this.getStorage();
    return storage ? storage.getItem(this.userKey) : null;
  }

  isAuthenticated(): boolean {
    return !!this.getToken();
  }

  private getStorage(): Storage | null {
    return typeof window !== 'undefined' ? window.localStorage : null;
  }
}
