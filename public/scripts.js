if (window.location.pathname == "/") {
    document.getElementById("loginForm").addEventListener("keydown", function(event) {
        if (event.key === "Enter") {
          event.preventDefault(); // Prevent form submission
          document.getElementById("loginButton").click(); // Trigger button click
        }
    });
}

let editing = false;
let editingIndex = -1;

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

    let i = 0;
    if (role == 3) {
        projects.forEach(project => {
            const projectElement = document.createElement("div");
            projectElement.classList.add("project");
            projectElement.setAttribute("id", `project${i}`);
            projectElement.innerHTML = `
            <button class="edit-button" onclick="editProject(${i})"><span class="material-symbols-outlined">edit</span></button>
            <h2>${project.name}</h2>
            <p>${project.description}</p>
            <p>Status: <b>${project.status}</b></p>
            `;
            container.appendChild(projectElement);
            i++;
        });
    }
    else {
        projects.forEach(project => {
            const projectElement = document.createElement("div");
            projectElement.classList.add("project");
            projectElement.setAttribute("id", `project${i}`);
            projectElement.innerHTML = `
            <h2>${project.name}</h2>
            <p>${project.description}</p>
            <p>Status: <b>${project.status}</b></p>
            `;
            container.appendChild(projectElement);
            i++;
        });
    }

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

async function editProject(index) {
    console.log(editing);
    console.log(editingIndex);
    if (editing) {
        if (editingIndex != index) {
            await cancelEdit(editingIndex);
        }
        else {
            await cancelEdit(index);
            return;
        }
    }

    editing = true;
    editingIndex = index;

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


    let oldName = document.getElementById(`project${index}`).getElementsByTagName("h2")[0].innerHTML;
    let oldDescription = document.getElementById(`project${index}`).getElementsByTagName("p")[0].innerHTML;
    let oldStatus = document.getElementById(`project${index}`).getElementsByTagName("p")[1].innerHTML;

    oldStatus = oldStatus.replace("Status: ", "");
    oldStatus = oldStatus.replace("<b>", "");
    oldStatus = oldStatus.replace("</b>", "");

    const projectElement = document.getElementById(`project${index}`);
    projectElement.innerHTML = `
    <button class="edit-button" onclick="editProject(${index})"><span class="material-symbols-outlined">edit</span></button>
    <br>
    <textarea class="edit-title" value="${oldName}" type="text">${oldName}</textarea>
    <input class="edit-description" value="${oldDescription}" type="text">
    <input class="edit-status" value="${oldStatus}" type="text">
    <br>
    <button class="edit-save" onclick="saveEditedProject(${index}, '${oldName}')">SAVE</button>
    <button class="edit-delete" onclick="deleteProject(${index})">DELETE</button>
    `;
}

async function cancelEdit(index) {
    if (!editing) {
        return;
    }

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

    fetch("/getProjects", {
        method: "GET",
        credentials: "include"
    })
    .then(response => response.json())  // Parse JSON directly
    .then(data => {
        project = data.projects[index];
        const projectElement = document.getElementById(`project${index}`);
        projectElement.innerHTML = `
        <button class="edit-button" onclick="editProject(${index})"><span class="material-symbols-outlined">edit</span></button>
        <h2>${project.name}</h2>
        <p>${project.description}</p>
        <p>Status: <b>${project.status}</b></p>
        `;
    })
    .catch(error => console.error("Error:", error));

    editing = false;
    editingIndex = -1;
}

async function saveEditedProject(index, oldName) {
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

    let newName = document.getElementById(`project${index}`).getElementsByTagName("textarea")[0].value;
    let newDescription = document.getElementById(`project${index}`).getElementsByTagName("input")[0].value;
    let newStatus = document.getElementById(`project${index}`).getElementsByTagName("input")[1].value;

    await fetch("/saveEditedProject", {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
        },
        body: JSON.stringify({ newName, newDescription, newStatus, oldName })
    })
    .then(response => response.text())  // Read the response as text
    .then(data => {
        if (data != "Project edited successfully!") {
            alert("Error editing project!");
        }
    })
    window.location.reload();
}

async function deleteProject(index) {
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

    await fetch("/deleteProject", {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
        },
        body: JSON.stringify({ projectName: document.getElementById(`project${index}`).getElementsByClassName("edit-title")[0].value })
    })
    .then(response => response.text())  // Read the response as text
    .then(data => {
        if (data != "Project deleted successfully!") {
            alert("Error deleting project!");
        }
    })
    window.location.reload();
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
