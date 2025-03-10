const express = require("express");
const path = require('path');
const fs = require('fs');
const app = express();

// Enable JSON body parsing (this should be before your routes)
app.use(express.json());  // This is needed to parse JSON data from the request body

// Serve static files from 'public' folder
app.use(express.static(path.join(__dirname, 'public')));

// Handle the POST request to '/loginRequest'
app.post("/loginRequest", (req, res) => {
    const { username } = req.body;  // Extract username from the request body

    // Read the users.json file
    const usersData = fs.readFileSync(path.join(__dirname, 'users.json'), 'utf8');
    // Parse the JSON data
    const users = JSON.parse(usersData).users;  // Assuming your JSON has a 'users' array
    
    // Find the user with the given username
    const user = users.find(user => user.name === username);
    if (user) {
        res.send(`User found: ${user.name}`);
    } else {
        res.send("User not found");
    }
});

/*
    TODO: Add password verification
    - Extract the password from the request body
    - Compare the password with the user's password
    - Send a response based on the password verification
    - Hash the password before storing it in the JSON and verify the hashed password
    - Use cookies to maintain user sessions
    - Implement a logout route to clear the session
    - Add error handling for invalid requests
*/ 

app.listen(3300, () => {
    console.log("Server is running on http://localhost:3300");
});