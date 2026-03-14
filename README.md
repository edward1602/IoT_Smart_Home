Project: IoT_smart_home — safe Git upload instructions

Steps to push repository without exposing keys:

1. Add local credentials (not committed):
   - For Gateway (Python): copy `.env.example` -> `.env` and set `AIO_USERNAME` and `AIO_KEY`.
   - For Web UI: copy `Web/web.config.example.js` -> `Web/web.config.js` and set credentials.

2. `.gitignore` already excludes `.env` and `web.config.js` so keys won't be committed.

3. Git push commands (example):

   git init
   git add .
   git commit -m "Initial commit (no secrets)"
   git remote add origin https://github.com/youruser/yourrepo.git
   git push -u origin main

Notes:
- Never commit real keys. Rotate keys if they were ever pushed.
- For the Web UI, putting the AIO key in client-side JS exposes it to users — prefer a server-side proxy.
