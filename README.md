# EVote Pakistan — Secure Digital Voting System

A single-page, front-end digital voting system web app that simulates a complete election workflow — voter registration, secure login, candidate voting, admin oversight, and live results — all built with plain HTML, CSS, and JavaScript.

## 📌 Overview

EVote Pakistan is a self-contained voting platform demo covering the full lifecycle of an election: voters register and get approved, an admin manages candidates and controls the election, voters log in and cast a single vote, and results are calculated and displayed live with analytics and export options.

## 🎯 Key Features

### Voter Side
- **Voter Registration** with CNIC (national ID) format validation and auto-formatting
- **Age Calculation & Eligibility Check** from date of birth
- **Secure Voter Login** with password strength checking
- **One Person, One Vote** enforcement — a voter can cast only a single vote per election
- **Candidate Selection & Vote Confirmation** flow (select → confirm → vote locked)
- **Voter Profile Management** (update profile, change password)
- **View Election Results** after casting a vote

### Admin Side
- **Separate Admin Login** with account lockout protection after repeated failed attempts
- **Admin Dashboard** with live KPIs (voter turnout, candidate stats, etc.)
- **Candidate Management** — add, edit, delete candidates
- **Voter Management** — approve, ban/unban, or remove registered voters
- **Election Control** — start and stop the election
- **Live Analytics & Vote Distribution** charts
- **Activity Logs** — track key actions, with export and clear options
- **Data Export** — export voters, results, and logs

### General
- **Inactivity Auto-Logout** for security (both voter and admin sessions)
- **Toast Notifications & Modals** for a smooth user experience
- **Fully Responsive**, dark-themed UI

## 🛠️ Tech Stack

- **HTML5** — page structure (single-page app with multiple internal views)
- **CSS3** — custom dark theme, CSS variables, responsive layout
- **Vanilla JavaScript** — all app logic (no frameworks)
- **Browser `localStorage`** — used as the data store (voters, candidates, votes, logs) — no external backend or database required
- **Font Awesome** & **Google Fonts** (Syne, DM Sans) — icons and typography

## 📂 Project Structure

This is a **single-file application** — everything (HTML, CSS, and JavaScript) is contained in:

```
EVote-Pakistan-v5.html
```

## 🚀 How to Run

No installation, server, or dependencies required.

1. Download `EVote-Pakistan-v5.html`
2. Open it directly in any modern web browser (Chrome, Firefox, Edge)
3. That's it — the app runs entirely client-side

> **Note:** Since data is stored in the browser's `localStorage`, all voter/candidate/results data persists only on the device and browser it was created in, and will reset if browser storage is cleared.

## 🖥️ App Flow

1. **Landing Page** — introduces the platform and live stats
2. **Voter Registration** — new voters sign up with CNIC and personal details
3. **Admin Login** → **Admin Dashboard** — admin approves voters, adds candidates, and starts the election
4. **Voter Login** — approved voters log in securely
5. **Cast Vote** — voter selects a candidate, confirms, and the vote is locked in
6. **Results** — live results and analytics available to both voters and admin

## ⚠️ Disclaimer

This project is a **front-end demo/prototype** built for learning and portfolio purposes. It uses browser `localStorage` for data persistence, which is **not suitable for a real-world election system** — a production system would require a secure backend, encrypted database, server-side authentication, and protection against tampering. This project should not be used for any real voting process.

## 🙋 Author

**Muhammad Saad **

## 📄 License

This project is open for learning purposes. Feel free to fork and build upon it.
