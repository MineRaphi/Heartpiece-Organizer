if (window.location.pathname == "/") {
    document.getElementById("loginForm").addEventListener("keydown", function(event) {
        if (event.key === "Enter") {
          event.preventDefault(); // Prevent form submission
          document.getElementById("loginButton").click(); // Trigger button click
        }
    });
}

async function login() {
    var username = document.getElementById("username").value;
    var password = document.getElementById("password").value;

    if (username == "" || password == "") {
        alert("Username or password cannot be empty!");
        return
    }

    username = username.toLowerCase();

    fetch("/loginRequest", {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
        },
        credentials: "include",  // This allows cookies to be set and sent
        body: JSON.stringify({ username, password }) 
    })
    .then(response => response.text())  // Read the response as text
    .then(data => {
        data = JSON.parse(data);
        if (data.success == true) {
            if (data.needToChangePassword == true) {
                changePassword(username);
            }
            document.getElementById("loginForm").reset();
            window.location.href = "/home/";
        }
        else {
            alert("Login failed!");
        }
    })  // Log the response from the backend
    .catch(error => console.error("Error:", error));  // Catch and log any errors
}

async function logout() {
    await fetch("/logout", {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
        },
        credentials: "include",  // This allows cookies to be set and sent
    })
    .then(response => response.text())  // Read the response as text
    .then(data => {
        window.location.href = "/";
    })  // Log the response from the backend
    .catch(error => console.error("Error:", error));  // Catch and log any errors
    window.location.href = "/";
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

async function checkUUID() {
    const response = await fetch("/checkUUID", {
        method: "GET",
        credentials: "same-origin", // Ensures cookies are sent with the request
    });

    if (response.status == 500) {
        if (window.location.pathname != "/") {
            window.location.href = "/";
        }
    }

    const data = await response.json();
    let userRole = data.user.role;

    if (!data.success) {
        if (window.location.pathname != "/") {
            window.location.href = "/";
        }
    }
    else {
        if (window.location.pathname == "/") {
            window.location.href = "/home/";
        }
        await fetch("/getProjects", {
            method: "GET",
            credentials: "include"
        })
        .then(response => response.json())  // Parse JSON directly
        .then(data => {
            displayProjects(data.projects, userRole); // Call function to display data
        })
        .catch(error => console.error("Error:", error));
    }
}

function displayProjects(projects, role) {
    const container = document.getElementById("projects");
    container.innerHTML = "";

    projects.forEach(project => {
        const projectElement = document.createElement("div");
        projectElement.classList.add("project");
        projectElement.innerHTML = `
            <h2>${project.name}</h3>
            <p>${project.description}</p>
            <p>Status: <b>${project.status}</b></p>
        `;
        container.appendChild(projectElement);
    });

    if (role == 3) {
        const projectElement = document.createElement("div");
        projectElement.classList.add("new-project");
        projectElement.innerHTML = `
            <h1>${"+"}</h1>
        `;
        projectElement.addEventListener("click", () => {
            createNewProject();
        });
        container.appendChild(projectElement);
    }
}

async function createNewProject() {
    const response = await fetch("/checkUUID", {
        method: "GET",
        credentials: "same-origin", // Ensures cookies are sent with the request
    });

    const data = await response.json();

    if (!data.success) {
        if (window.location.pathname != "/") {
            window.location.href = "/";
        }
    }
    else if (data.user.role != 3) {
        alert("Error: Permission denied!");
        return;
    }
    
    const projectName = prompt("Enter the name of the new project:");
    if (projectName == null || projectName == "") {
        return;
    }
    const projectDescription = prompt("Enter the description of the new project:");
    if (projectDescription == null || projectDescription == "") {
        return;
    }
    const projectStatus = prompt("Enter the status of the new project:");
    if (projectStatus == null || projectStatus == "") {
        return;
    }
    await fetch("/createNewProject", {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
        },
        body: JSON.stringify({ projectName, projectDescription, projectStatus })
    })
    .then(response => response.text())  // Read the response as text
    .then(data => {
        if (data != "Project created successfully!") {
            alert("Error creating project!");
        }
    })
    window.location.reload();
}

// Call the function to check the UUID
checkUUID();
