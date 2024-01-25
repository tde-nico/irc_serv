-- psql -h localhost -U postgres -d irc -f schema.sql
-- sudo -u postgres psql

CREATE TABLE logs (
	id SERIAL PRIMARY KEY,
	log VARCHAR(1023),
	date TIMESTAMP
);

CREATE TABLE channels (
	name VARCHAR(255) PRIMARY KEY,
	password VARCHAR(255) NULL
);

