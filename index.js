const express = require('express');
const path = require('path');

const app = express();

// Serve static files from 'public' folder
app.use(express.static(path.join(__dirname, 'public')));

app.listen(3300, () => {
    console.log('Server is running on http://localhost:3000');
});