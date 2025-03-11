const express = require("express");
const path = require("path");
const fs = require("fs");
const https = require("https");

const app = express();

// Load SSL certificate and private key
const sslOptions = {
    key: fs.readFileSync("server.key"),
    cert: fs.readFileSync("server.cert")
};

// Enable JSON body parsing
app.use(express.json());

// Serve static files
app.use(express.static(path.join(__dirname, "public")));

// Handle the POST request to '/loginRequest'
app.post("/loginRequest", (req, res) => {
    const { username, password } = req.body;

    // Read users.json
    const usersData = fs.readFileSync(path.join(__dirname, "users.json"), "utf8");
    const users = JSON.parse(usersData).users;

    // Find user
    const user = users.find(user => user.name === username);
    if (!user) return res.send("User not found");

    // Check password
    if (user.password !== password) return res.send("Incorrect password");

    res.send(`Login successful! Welcome, ${user.name}`);
});

const http = require("http");

http.createServer((req, res) => {
    res.writeHead(301, { "Location": "https://" + req.headers.host + req.url });
    res.end();
}).listen(80, () => {
    console.log("Redirecting HTTP to HTTPS...");
});

// Start HTTPS server on port 443
https.createServer(sslOptions, app).listen(443, () => {
    console.log("HTTPS Server running on https://localhost");
});