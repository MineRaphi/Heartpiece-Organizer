document.getElementById("loginForm").addEventListener("keydown", function(event) {
    if (event.key === "Enter") {
      event.preventDefault(); // Prevent form submission
      document.getElementById("loginButton").click(); // Trigger button click
    }
});

async function login() {
    var username = document.getElementById("username").value;
    var password = document.getElementById("password").value;

    if (username == "" || password == "") {
        alert("Username or password cannot be empty!");
        return
    }

    fetch("/loginRequest", {
        method: "POST",
        headers: {
            "Content-Type": "application/json",  // Ensure the content type is JSON
        },
        body: JSON.stringify({ username: `${username}`, password: `${password}` })  // Send username in the request body
    })
    .then(response => response.text())  // Read the response as text
    .then(data => {
        data = JSON.parse(data);
        if (data.success == true) {
            alert("Login successful!");
            if (data.needToChangePassword == true) {
                changePassword(username);
            }
            document.getElementById("loginForm").reset();
        }
        else {
            alert("Login failed!");
        }
    })  // Log the response from the backend
    .catch(error => console.error("Error:", error));  // Catch and log any errors
}

function changePassword(username) {
    let newPassword = "";
    while (newPassword == null || newPassword == "") {
        newPassword = prompt("Please enter a new password:");
    }

    fetch("/changePassword", {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
        },
        body: JSON.stringify({ username: `${username}`, newPassword: `${newPassword}` })
    });
}
