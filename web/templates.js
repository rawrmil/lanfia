templReadyNextButton =
	`
		<div style="display: flex; justify-content: center;">
			<button id="ready_next_button" onclick="SendGameReadyNext()">
				Ready
			</button>
		</div>
	`

templPlayerListEntry =
	`
		<div class='player_entry'>
			<span class='player_number'>\${Number(i)+1}.</span>
			&nbsp;
			<span class='player_name'>\${Escaped(player_names[i]+"\\n")}</span>
			&nbsp;
			<span class='player_name'>\${player_states[i] != "0" ? "(ready)" : ""}</span>
		</div>
	`

templGameStarted = `<h1>GAME STARTED</h1>`

templGameStartedPlayerNote =
	`
		<p>
			Become familiar with your role and let's get started!
		</p>
		<p>
			Press "Ready" button
		</p>
	`

templGameStartedSpectatorNote = `<p>You are a spectator!</p>`


templRoleVillager = `villager`;
templRoleMafia = `mafia`;
templRoleSerif = `serif`;
templRoleDoctor = `doctor`;
templRoleEscort = `escort`;
templRoleManiac = `maniac`;
templGameActionRole = `<span>Your role is <b>\${role_name}</b>!</span>`

templDayStarted = `<h3>DAY STARTED</h3>`
templNightStarted = `<h3>NIGHT STARTED</h3>`

templChoicesNote = `<p>Players are doing their choices!</p>`

const templRoleTasks = Object.freeze({
    [GRT.VILLAGER]: `
        <h3>Villager Task</h3>
        <p>Just stare at the screen bro</p>
    `,
    [GRT.MAFIA]: `
        <h3>Mafia Task</h3>
        <p>There will be a poll</p>
    `,
    [GRT.SERIF]: `
        <h3>Serif Task</h3>
        <p>There will be a poll with response</p>
    `,
    [GRT.DOCTOR]: `
        <h3>Doctor Task</h3>
        <p>There will be a poll</p>
    `,
    [GRT.ESCORT]: `
        <h3>Escort Task</h3>
        <p>There will be a poll</p>
    `,
    [GRT.MANIAC]: `
        <h3>Maniac Task</h3>
        <p>There will be a poll</p>
    `
});

templGameEnded = `<h1>GAME RESULTS</h1>`;
