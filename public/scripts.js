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
        body: JSON.stringify({ username: `${username}` })  // Send username in the request body
    })
    .then(response => response.text())  // Read the response as text
    .then(data => console.log(data))  // Log the response from the backend
    .catch(error => console.error("Error:", error));  // Catch and log any errors
}
