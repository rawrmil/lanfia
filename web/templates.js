const is_ru = /^ru\b/i.test(navigator.language || navigator.userLanguage);

function EN(s) {
	return is_ru ? "" : s;
}

function RU(s) {
	return is_ru ? s : "";
}

templReadyNextButton =
    `
        <div style="display: flex; justify-content: center;">
            <button id="ready_next_button" onclick="SendGameReadyNext()">
                \${EN("Ready")}\${RU("Готов")}
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
            <span class='player_name'>\${GetPlayerStatus(i)}</span>
        </div>
    `

templGameStarted = `<h1>\${EN("GAME STARTED")}\${RU("ИГРА НАЧАЛАСЬ")}</h1>`

templGameStartedPlayerNote =
    `
        <p>
            \${EN("Become familiar with your role and let's get started!")}
            \${RU("Ознакомьтесь со своей ролью и начинаем!")}
        </p>
        <p>
            \${EN("Press \"Ready\" button")}
            \${RU("Нажмите кнопку \"Готов\"")}
        </p>
    `

templGameStartedSpectatorNote = `<p>\${EN("You are a spectator!")}\${RU("Вы наблюдатель!")}</p>`


templGameActionRole = `<span>\${EN("Your role is")}\${RU("Ваша роль")} <b>\${role_name}</b>!</span>`

templDayStarted = `<h3>\${EN("DAY STARTED")}\${RU("ДЕНЬ НАЧАЛСЯ")}</h3>`
templNightStarted = `<h3>\${EN("NIGHT STARTED")}\${RU("НОЧЬ НАЧАЛАСЬ")}</h3>`

templChoicesNote = `<p>\${EN("Players are doing their choices!")}\${RU("Игроки делают выбор!")}</p>`

const templRoleNames = Object.freeze({
    [GRT.TOWNSMAN]: `\${EN("villager")}\${RU("мирный житель")}`,
    [GRT.MAFIA]: `\${EN("mafia")}\${RU("мафия")}`,
    [GRT.SERIF]: `\${EN("serif")}\${RU("шериф")}`,
    [GRT.DOCTOR]: `\${EN("doctor")}\${RU("доктор")}`,
    [GRT.ESCORT]: `\${EN("escort")}\${RU("эскорт")}`,
    [GRT.MANIAC]: `\${EN("maniac")}\${RU("маньяк")}`
});

const templRoleTasks = Object.freeze({
    [GRT.TOWNSMAN]: `
        <h3>\${EN("Villager Task")}\${RU("Задача мирного жителя")}</h3>
        <p>\${EN("Just stare at the screen bro")}\${RU("Просто смотри в экран, братан")}</p>
    `,
    [GRT.MAFIA]: `
        <h3>\${EN("Mafia Task")}\${RU("Задача мафии")}</h3>
        <p>\${EN("Last Mafia to vote locks the kill target")}\${RU("Последний мафия, проголосовавший, выбирает цель убийства")}</p>
    `,
    [GRT.SERIF]: `
        <h3>\${EN("Serif Task")}\${RU("Задача шерифа")}</h3>
    `,
    [GRT.DOCTOR]: `
        <h3>\${EN("Doctor Task")}\${RU("Задача доктора")}</h3>
    `,
    [GRT.ESCORT]: `
        <h3>\${EN("Escort Task")}\${RU("Задача эскорта")}</h3>
    `,
    [GRT.MANIAC]: `
        <h3>\${EN("Maniac Task")}\${RU("Задача маньяка")}</h3>
    `
});

templGameEnded = `<h1>\${EN("GAME RESULTS")}\${RU("РЕЗУЛЬТАТЫ ИГРЫ")}</h1>`;

templGamePoll =
    `
        <div id="game_poll">
            \${EN("Choose:")}\${RU("Выберите:")}
            \${GeneratePoll()}
        </div>
    `;

const templRoleChose = Object.freeze({
    [GRT.TOWNSMAN]: `
    `,
    [GRT.MAFIA]: `
            \${EN("Anonymous note: Player")}\${RU("Анонимная записка: Игрок")} '\${player_names[voter_index]}' \${EN("chose to kill")}\${RU("выбрал убить")}
            '\${player_names[chosen_index]}'
    `,
    [GRT.SERIF]: `
            \${EN("Anonymous note: Player")}\${RU("Анонимная записка: Игрок")} '\${player_names[voter_index]}' \${EN("chose to check")}\${RU("выбрал проверить")} 
            '\${player_names[chosen_index]}'
    `,
    [GRT.DOCTOR]: `
            \${EN("Anonymous note: Player")}\${RU("Анонимная записка: Игрок")} '\${player_names[voter_index]}' \${EN("chose to heal")}\${RU("выбрал лечить")}
            '\${player_names[chosen_index]}'
    `,
    [GRT.ESCORT]: `
            \${EN("Anonymous note: Player")}\${RU("Анонимная записка: Игрок")} '\${player_names[voter_index]}' \${EN("chose to stunn")}\${RU("выбрал оглушить")}
            '\${player_names[chosen_index]}'
    `,
    [GRT.MANIAC]: `
            \${EN("Anonymous note: Player")}\${RU("Анонимная записка: Игрок")} '\${player_names[voter_index]}' \${EN("chose to kill")}\${RU("выбрал убить")}
            '\${player_names[chosen_index]}'
    `,
    255: `
            \${EN("Player")}\${RU("Игрок")} '\${player_names[voter_index]}' \${EN("chose to kick")}\${RU("выбрал выгнать")}
            '\${player_names[chosen_index]}'
        `
});

templPlayerKilled =
    `
        \${EN("Player")}\${RU("Игрок")} '\${player_names[chosen_index]}' \${EN("killed.")}\${RU("убит.")}
    `;

templPlayerChecked =
    `
        \${EN("Anonymous note: Player")}\${RU("Анонимная записка: Игрок")} '\${player_names[chosen_index]}' \${EN("is")}\${RU("это")} \${templRoleNames[role]}.
    `;

templPlayerKicked =
    `
        \${EN("Player")}\${RU("Игрок")} '\${player_names[chosen_index]}' \${EN("kicked.")}\${RU("выгнан.")}
    `;

templPlayerHealed =
    `
        \${EN("Player")}\${RU("Игрок")} '\${player_names[chosen_index]}' \${EN("healed.")}\${RU("вылечен.")}
    `;

templPlayerStunned =
    `
        \${EN("Player")}\${RU("Игрок")} '\${player_names[chosen_index]}' \${EN("stunned.")}\${RU("оглушен.")}
    `;

templGameMafiaGameStarted = `\${EN("Mafia (game started)")}\${RU("Мафия (игра началась)")}`;

templGameMafiaGameEnded = `\${EN("Mafia (game ended)")}\${RU("Мафия (игра окончена)")}`;

templYouCanReload = `<p>\${EN("You can reload the page!")}\${RU("Можно перезагрузить страницу!")}</p>`

templMafiaWon = `<h1>\${EN("MAFIA WON")}\${RU("МАФИЯ ПОБЕДИЛА")}</h1>`;
templTownWon = `<h1>\${EN("TOWN WON")}\${RU("ГОРОД ПОБЕДИЛ")}</h1>`;
templManiacWon = `<h1>\${EN("MANIAC WON")}\${RU("МАНЬЯК ПОБЕДИЛ")}</h1>`;

templErrGameStarted = `\${EN("Game already started")}\${RU("Игра уже началась")}`;
templErrNameTooLong = `\${EN("Name too long")}\${RU("Имя слишком длинное")}`;
templTimer = `\${EN("Timer: ")}\${RU("Таймер: ")}`;
