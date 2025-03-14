const express = require("express");
const bcrypt = require("bcrypt");
const path = require("path");
const fs = require("fs");
const https = require("https");
const cookieParser = require("cookie-parser");
const crypto = require("crypto");

const saltRounds = 10;
const app = express();

app.use(cookieParser());

// Load SSL certificate and private key
const sslOptions = {
    key: fs.readFileSync("server.key"),
    cert: fs.readFileSync("server.cert")
};

// Enable JSON body parsing
app.use(express.json());

// Serve static files
app.use(express.static(path.join(__dirname, "public")));

app.post("/changePassword", async (req, res) => {
    const { username, newPassword } = req.body;

    try {
        let hashedPassword = await genHash(newPassword, saltRounds);

        // Read users.json
        const usersFilePath = path.join(__dirname, "users.json");
        const usersData = fs.readFileSync(usersFilePath, "utf8");
        const usersObj = JSON.parse(usersData);
        const users = usersObj.users;

        // Find user
        const user = users.find(user => user.name === username);
        if (!user) return res.send("User not found");

        // Update password
        user.password = hashedPassword;

        // Write back to file
        fs.writeFileSync(usersFilePath, JSON.stringify(usersObj, null, 4), "utf8");

        res.send("Password successfully changed!");
    } catch (err) {
        res.status(500).send("Error changing password");
    }
});

// Handle the POST request to '/loginRequest'
app.post("/loginRequest", async (req, res) => {
    const { username, password } = req.body;

    // Read users.json
    const usersData = fs.readFileSync(path.join(__dirname, "users.json"), "utf8");
    const users = JSON.parse(usersData).users;

    // Find user
    const user = users.find(user => user.name === username);
    
    if (!user) return res.json({ "success": false });

    // Check if user has the default password and needs to change it
    if (password == "heartpiecePassword1121" && user.password == "heartpiecePassword1121") {
        res.json({ success: true, needToChangePassword: true });
        return;
    }

    let samePassword = await bcrypt.compare(password, user.password);

    // Compare password
    if (samePassword) {
        res.json({ success: true });
    }
    else {
        res.json({ success: false });
    }
});

async function genHash(password, saltRounds) {
    try {
        return await bcrypt.hash(password, saltRounds);
    } catch (err) {
        console.error("Error hashing password:", err);
        throw err; // Re-throw the error for better error handling
    }
}

async function genUUID(username) {
    let uuid = crypto.randomUUID();
    
}

const http = require("http");
const { hash } = require("crypto");

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