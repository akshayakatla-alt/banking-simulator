# SJNA Bank Transaction Simulator

## Run locally with Docker Compose

1. Install and start Docker Desktop.
2. Copy `.env.example` to `.env` and set your local database password and JWT secret.
3. From the project root, run:

   ```powershell
   docker compose up --build
   ```

4. Open the frontend at <http://localhost:8081>. The API is available at
   <http://localhost:8080>, and its health endpoint is <http://localhost:8080/api/health>.

Compose starts PostgreSQL, waits for it to become healthy, initializes a new
database with `backend/sql/schema.sql`, builds and starts the API, and serves the
frontend. PostgreSQL data is kept in the `postgres_data` volume between runs.
Schema initialization runs only when that volume is first created.

To stop the services without deleting database data, run
`docker compose down`. To intentionally remove the local database volume as
well, run `docker compose down --volumes`; this permanently deletes its data.

The default credentials in Compose are for local development only. Do not expose
this configuration to an untrusted network or use it for production.
