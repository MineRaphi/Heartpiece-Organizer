// allows the user to press enter to submit the login form
if (window.location.pathname == "/") {
    document.getElementById("loginForm").addEventListener("keydown", function(event) {
        if (event.key === "Enter") {
          event.preventDefault(); // Prevent form submission
          document.getElementById("loginButton").click(); // Trigger button click
        }
    });
}

window.addEventListener("DOMContentLoaded", () => {
    openProject(window.location.hash.substring(1));
});

window.addEventListener("hashchange", () => {
    openProject(window.location.hash.substring(1));
});

window.addEventListener("resize", () => {
    const table = document.getElementById('statusList');
    const button = document.getElementById("sceneAddButton");

    const cell = table.rows[2+currentProjectScenes*2-1].cells[0];
    const rect = cell.getBoundingClientRect();

    button.style.left = rect.left + 17 + "px";
    button.style.top = rect.bottom - 7 + "px";
});

window.addEventListener("resize", () => {
    const table = document.getElementById('statusList');
    const cutAddButton = document.getElementById("cutAddButton");

    const cutCell = table.rows[2+currentProjectScenes*2-1].cells[0];
    const cutRect = cutCell.getBoundingClientRect();

    cutAddButton.style.position = "absolute";
    cutAddButton.style.left = cutCell.left + 17 + "px";
    cutAddButton.style.top = cutRect.bottom - 7 + "px";
});

document.addEventListener("mousemove", (e) => {
    const sceneAddButton = document.getElementById("sceneAddButton");
    changeOpacityWithCursor(sceneAddButton, e.clientX, e.clientY, 1, 40);
    const cutAddButton = document.getElementById("cutAddButton");
    changeOpacityWithCursor(cutAddButton, e.clientX, e.clientY, 1, 40);
});

// global variables
let editing = false;
let editingIndex = -1;
const statusOptions = ["Approval pending", "Approved", "Pre-Production", "Production", "Post-Production", "Finished", "Released"];
let newProjectScenes = 0;
let newProjectCuts = [];
let currentProjectScenes = 0;
let currentProjectCuts = [];

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
        projects.forEach((project, i) => {
            const projectElement = document.createElement("div");
            projectElement.classList.add("project");
            projectElement.onclick = () => {
                window.location.hash = `#${i}`;
            };
            projectElement.setAttribute("id", `project${i}`);
            projectElement.innerHTML = `
            <button class="edit-button" onclick="editProject(event, ${i})"><span class="material-symbols-outlined">edit</span></button>
            <h2>${project.name}</h2>
            <p>${project.description}</p>
            <p>Status: <b>${project.status}</b></p>
            `;
            container.appendChild(projectElement);
        });
    }
    else {
        projects.forEach(project => {
            const projectElement = document.createElement("div");
            projectElement.classList.add("project");
            projectElement.setAttribute("id", `project${i}`);
            projectElement.onclick = () => {
                window.location.hash = `#${i}`;
            };
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
        projectElement.setAttribute("id", `newProject`);
        projectElement.innerHTML = `
            <h1>${"+"}</h1>
        `;
        projectElement.addEventListener("click", createNewProject);
        container.appendChild(projectElement);
    }
}

async function editProject(event, index) {
    event.stopPropagation();
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
        window.location.href = "/home/";
        return;
    }


    let oldName = document.getElementById(`project${index}`).getElementsByTagName("h2")[0].innerHTML;
    let oldDescription = document.getElementById(`project${index}`).getElementsByTagName("p")[0].innerHTML;
    let oldStatus = document.getElementById(`project${index}`).getElementsByTagName("p")[1].innerHTML;

    oldStatus = oldStatus.replace("Status: ", "");
    oldStatus = oldStatus.replace("<b>", "");
    oldStatus = oldStatus.replace("</b>", "");

    const select = document.createElement("select");
    select.classList.add("edit-status");
    statusOptions.forEach(option => {
        const opt = document.createElement("option");
        opt.value = option;
        opt.text = option;
        select.appendChild(opt);
    });

    const projectElement = document.getElementById(`project${index}`);
    projectElement.innerHTML = `
        <button class="edit-button" onclick="editProject(event, ${index})"><span class="material-symbols-outlined">cancel</span></button>
        <br>
        <textarea class="edit-title" value="${oldName}" type="text">${oldName}</textarea>
        <input class="edit-description" value="${oldDescription}" type="text">
    `;
    projectElement.appendChild(select);
    projectElement.innerHTML += `
        <br>
        <button class="edit-save material-symbols-outlined" onclick="saveEditedProject(event, ${index}, '${oldName}')">save</button>
        <button class="edit-delete material-symbols-outlined" onclick="deleteProject(event, ${index})">delete_forever</button>
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
        window.location.href = "/home/";
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
        <button class="edit-button" onclick="editProject(event, ${index})"><span class="material-symbols-outlined">edit</span></button>
        <h2>${project.name}</h2>
        <p>${project.description}</p>
        <p>Status: <b>${project.status}</b></p>
        `;
    })
    .catch(error => console.error("Error:", error));

    editing = false;
    editingIndex = -1;
}

async function saveEditedProject(event, index, oldName) {
    event.stopPropagation();
    const response = await fetch("/checkUUID", {
        method: "GET",
        credentials: "same-origin",
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
    let newStatus = document.getElementById(`project${index}`).getElementsByTagName("select")[0].value;

    await fetch("/saveEditedProject", {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
        },
        body: JSON.stringify({ newName, newDescription, newStatus, oldName })
    })
    .then(response => response.text())
    .then(data => {
        if (data != "Project edited successfully!") {
            alert("Error editing project!");
        }
    })
    window.location.reload();
}

async function deleteProject(event, index) {
    event.stopPropagation();

    if (!editing) {
        return;
    }

    if (!confirm("Are you sure you want to delete this project?")) {
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

    const select = document.createElement("select");
    select.classList.add("edit-status");
    statusOptions.forEach(option => {
        const opt = document.createElement("option");
        opt.value = option;
        opt.text = option;
        select.appendChild(opt);
    });

    const projectElement = document.getElementById(`newProject`);
    projectElement.className = "project";
    projectElement.removeEventListener("click", createNewProject);
    projectElement.innerHTML = `
    <button class="edit-button" onclick="cancelNewProject(event)"><span class="material-symbols-outlined">cancel</span></button>
    <br>
    <textarea class="edit-title" value="" placeholder="Project Title" type="text"></textarea>
    <input class="edit-description" value="" placeholder="Project Description" type="text">
    `;
    projectElement.appendChild(select);
    projectElement.innerHTML += `
    <br>
    <button class="edit-save material-symbols-outlined" onclick="saveNewProject(event)">save</button>
    <button class="edit-delete material-symbols-outlined" onclick="cancelNewProject(event)">cancel</button>
    `;
}

async function cancelNewProject(event) {
    event.stopPropagation();
    projectElement = document.getElementById(`newProject`);
    projectElement.className = "new-project";
    projectElement.innerHTML = `
        <h1>${"+"}</h1>
    `;
    projectElement.addEventListener("click", createNewProject);
}

async function saveNewProject(event) {
    event.stopPropagation();
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
        window.location.href = "/home/";
        return;
    }

    let projectName = document.getElementById(`newProject`).getElementsByTagName("textarea")[0].value;
    let projectDescription = document.getElementById(`newProject`).getElementsByTagName("input")[0].value;
    let projectStatus = document.getElementById(`newProject`).getElementsByTagName("select")[0].value;
    if (projectName == "" || projectDescription == "") {
        alert("Project name or description cannot be empty!");
        return;
    }
    projectName = projectName.replace("\n", " ");
    projectDescription = projectDescription.replace("\n", " ");
    projectStatus = projectStatus.replace("\n", " ");

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

async function openProject(index) {
    if (index == "" || isNaN(index)) {
        return;
    }
    if (editing) {
        window.location.hash = `#`;
        return;
    }

    loadProjectInfo(index);

    document.getElementById("projects").style.display = "none";
    document.getElementById("projectInfo").style.display = "block";

}

async function loadProjectInfo(index) {
    await fetch("/getProjects", {
        method: "GET",
        credentials: "include"
    })
    .then(response => response.json())  // Parse JSON directly
    .then(data => {
        document.getElementById("infoProjectName").innerHTML = data.projects[index].name;
        document.getElementById("infoProjectDescription").innerHTML = data.projects[index].description;
    })
    .catch(error => console.error("Error:", error));

    await fetch(`/getProjectDetails?projectIndex=${index}`, {
        method: "GET",
        credentials: "include"
    })
    .then(response => response.json())  // Parse JSON directly
    .then(data => {
        const table = document.getElementById('statusList');
        const cols = 18;
        const rows = data.scenes.reduce((sum, scene) => sum + scene.cuts.length, 0);
        const sceneCount = data.scenes.length;
        currentProjectScenes = sceneCount;
        let sceneCuts = [];

        table.innerHTML = `
        <thead>
            <tr>
                <th colspan="2"></th>
                <th colspan="4">Pre-Production</th>
                <th colspan="4">Production</th>
                <th colspan="6">Post-Production</th>
                <th colspan="2">Distribute</th>
            </tr>
            <tr>
                <th>Scenes</th>
                <th>Cuts</th>
                <th>Script</th>
                <th>Storyboard</th>
                <th>Animatic</th>
                <th>Voice Acting</th>
                <th>Rough Animation</th>
                <th>Cleanup</th>
                <th>Coloring</th>
                <th>Background</th>
                <th>FX-Animation</th>
                <th>Compositing</th>
                <th>Music</th>
                <th>Sound Design</th>
                <th>Mix/Master</th>
                <th>Render</th>
                <th>Dubbing</th>
                <th>Localisation</th>
            </tr>
        </thead>
        `;

        for(let i=0; i<sceneCount; i++) {
            sceneCuts[i] = data.scenes[i].cuts.length;
        }

        currentProjectCuts = sceneCuts;

        for (let i = 0; i < sceneCount; i++) {
            const row = table.insertRow();
            const cell = row.insertCell();
            cell.className = "spacer-row";
            cell.colSpan = 18;
            for (let j = 0; j < sceneCuts[i]; j++) {
                const row = table.insertRow();
                for (let k = 0; k < cols; k++) {
                    if (j==0 && k==0) {
                        const cell = row.insertCell();
                        cell.rowSpan = sceneCuts[i];
                        cell.innerHTML = i+1;
                    }
                    else if (j!=0 && k==0 || j!=0 && k==2) {
                        continue;
                    }
                    else {
                        if (k==1) {
                            const cell = row.insertCell();
                            cell.innerHTML = j+1;
                        }
                        else if (k==2 && j==0) {
                            const cell = row.insertCell();
                            cell.rowSpan = sceneCuts[i];
                            setCellColor(cell, data.scenes[i].script);
                            cell.onclick = () => {
                                changeCellState(cell, data.scenes[i].script);
                            };
                        }
                        else {
                            const cell = row.insertCell();
                            setCellColor(cell, data.scenes[i].cuts[j][k]);
                            cell.onclick = () => {
                                changeCellState(cell, data.scenes[i].cuts[j][k]);
                            };
                        }
                    }
                }
            }
        }

        const sceneAddButton = document.getElementById("sceneAddButton");

        const sceneCell = table.rows[2+currentProjectScenes*2-1].cells[0];
        const sceneRect = sceneCell.getBoundingClientRect();

        sceneAddButton.style.position = "absolute";
        sceneAddButton.style.left = sceneRect.left + 17 + "px";
        sceneAddButton.style.top = sceneRect.bottom - 7 + "px";
        
        sceneAddButton.onclick = () => {
            addScene(table, sceneCount);
        };


        const cutAddButton = document.getElementById("cutAddButton");

        const cutCell = table.rows[2+currentProjectScenes*2-1].cells[1];
        const cutRect = cutCell.getBoundingClientRect();

        cutAddButton.style.position = "absolute";
        cutAddButton.style.left = cutRect.left + 5 + "px";
        cutAddButton.style.top = cutRect.bottom - 7 + "px";
        
        cutAddButton.onclick = () => {
            alert("WIP");
        };
        
    })
    .catch(error => console.error("Error:", error));
}

function setCellColor(cell, colorId) {
    if (colorId == 0 || colorId == undefined || isNaN(colorId)) {
        cell.style.background = "gray";
    }
    else if (colorId == 1) {
        cell.style.background = "red";
    }
    else if (colorId == 2) {
        cell.style.background = "yellow";
    }
    else if (colorId == 3) {
        cell.style.background = "green";
    }
}

function changeCellState(cell, currentColor) {
    if (isNaN(currentColor)) {
        currentColor = 0;
    }
    let color = currentColor+1;
    if (color>3) {
        color = 1;
    }
    setCellColor(cell, color);
    cell.onclick = () => {
        changeCellState(cell, color);
    };
}

async function saveProjectState() {
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

    const index = window.location.hash.substring(1);
    let saveData;
    
    await fetch(`/getProjectDetails?projectIndex=${index}`, {
        method: "GET",
        credentials: "include"
    })
    .then(response => response.json())
    .then(data => {
        const table = document.getElementById('statusList');
        const cols = 18;
        const rows = data.scenes.reduce((sum, scene) => sum + scene.cuts.length, 0);
        const sceneCount = data.scenes.length + newProjectScenes;
        let sceneCuts = [];

        saveData = data;

        for(let i=0; i<sceneCount; i++) {
            if (i<sceneCount-newProjectScenes) {
                sceneCuts[i] = data.scenes[i].cuts.length;
            }
            else {
                sceneCuts[i] = newProjectScenes[i-(sceneCount-newProjectScenes)];
            }
        }

        for(let i=0; i<newProjectCuts.length; i++) {
            console.log(sceneCuts);
            sceneCuts.pop();
            sceneCuts.push(newProjectCuts[i]);
        }

        console.log(sceneCuts);

        while (saveData.scenes.length < sceneCount) {
            saveData.scenes.push({ "script": 0, "cuts": [[
                null,
                null,
                null,
                1,
                1,
                1,
                1,
                1,
                1,
                1,
                1,
                1,
                1,
                1,
                1,
                1,
                1,
                1
            ]] });
        }

        saveData.scenes[0].script = getIdFromColor(table.rows[3].cells[2].style.background);

        for(let i=0, j=3; i<sceneCount; i++) {
            saveData.scenes[i].script = getIdFromColor(table.rows[j].cells[2].style.background);
            j+=sceneCuts[i]+1;
        }

        let row=3;
        for(let i=0; i<sceneCount; i++) {
            console.log(sceneCuts[i]);
            for(let j=0; j<sceneCuts[i]; j++) {
                let cols = 16;
                let k = 1;
                if(j==0) {
                    cols+=2;
                    k+=2
                }
                for(; k<cols; k++) {
                    if (!table.rows[row]) {
                        console.warn(`Row ${row} does not exist`);
                    }
                    if (!table.rows[row].cells[k]) {
                        console.warn(`Cell ${k} does not exist in row ${row}`);
                    }
                    if (j==0) {
                        saveData.scenes[i].cuts[j][k] = getIdFromColor(table.rows[row].cells[k].style.background);
                    }
                    else {
                        saveData.scenes[i].cuts[j][k+2] = getIdFromColor(table.rows[row].cells[k].style.background);
                    }
                }
                row++;
            }
            row++;
        }
    });

    console.log(saveData);

    await fetch("/saveProjectDetails", {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
        },
        body: JSON.stringify({ saveData, index })
    })
    .then(response => response.text())
    .then(data => {
        if (data == "Success") {
            alert("Project Details successfully saved!");
            window.location.reload();
            newProjectScenes = 0;
            newProjectCuts = [];
        }
        else {
            alert("Error saving Project Details!");
        }
    });
}

function getIdFromColor(color) {
    if (color == "red") {
        return 1;
    }
    else if (color == "yellow") {
        return 2;
    }
    else if (color == "green") {
        return 3;
    }
    else {
        return 0;
    }
}

async function closeProjectInfo() {
    window.location.hash = `#`;
    window.location.reload();
}

function addScene(table) {
    const spacerRow = table.insertRow();
    const spacerCell = spacerRow.insertCell();
    spacerCell.className = "spacer-row";
    spacerCell.colSpan = 18;

    const newRow = table.insertRow();
    const sceneCell = newRow.insertCell();
    sceneCell.innerHTML = currentProjectScenes+1;
    const cutCell = newRow.insertCell();
    cutCell.innerHTML = "1";
    for(let i=0; i<16; i++) {
        const newCell = newRow.insertCell();
        setCellColor(newCell, 1);
        newCell.onclick = () => {
            changeCellState(newCell, 1);
        }
    }
    newProjectScenes++;
    currentProjectScenes++;
    newProjectCuts[newProjectCuts.length] = 1;

    const button = document.getElementById("sceneAddButton");

    const cell = table.rows[2+currentProjectScenes*2-1].cells[0];
    const rect = cell.getBoundingClientRect();

    button.style.left = rect.left + 17 + "px";
    button.style.top = rect.bottom - 7 + "px";

}

function addCut(table) {

}

function changeOpacityWithCursor(elem, mouseX, mouseY, minRadius, maxRadius) {
    const rect = elem.getBoundingClientRect();
    const centerX = rect.left + rect.width / 2;
    const centerY = rect.top + rect.height / 2;
    let value = 0;

    const dx = mouseX - centerX;
    const dy = mouseY - centerY;
    const distance = Math.sqrt(dx * dx + dy * dy);

    if (distance <= minRadius) {
        value = 0;
    };

    if (distance >= maxRadius) {
        value = 100;
    };

    value = ((distance - minRadius) / (maxRadius - minRadius)) * 100;
    value = 100 - value;

    elem.style.opacity = value / 100;
}

// Call the function to check the UUID
checkUUID();
