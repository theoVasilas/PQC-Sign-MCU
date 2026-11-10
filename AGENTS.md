# Git workflow preferences

- Help the user learn Git with short, plain-language explanations tied to the current work.
- Inspect the working tree before changing files, preserving existing user changes.
- Remind the user to commit after a meaningful, verified change, before a substantial experiment or refactor, and at the end of a work session when changes remain. Avoid repeating a reminder when nothing relevant has changed.
- At a commit checkpoint, summarize the relevant changes, report validation performed, and suggest a concise commit message and the specific files to stage.
- Prefer small commits covering one logical change. Review the diff and stage explicit paths rather than indiscriminately staging everything.
- Commit or push only when the user requests it; a request for reminders alone does not authorize either action.
- Help with branches, diffs, history, conflicts, and recovery when relevant. Preserve uncommitted work and avoid destructive operations or rewriting shared history without explicit authorization.
- Treat external/PQClean as a separate dependency repository. Run Git commands from the intended repository and clearly distinguish its changes from those in PQC-Sign-MCU. Do not commit, push, or change its Git metadata unless requested.
- Reminders are conversational checkpoints during active work, not scheduled background notifications.
