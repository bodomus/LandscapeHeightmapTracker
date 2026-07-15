# LandscapeHeightmapTracker Codex workflow package

Copy the files into the repository root while preserving paths:

```text
LandscapeHeightmapTracker/
├── AGENTS.md
├── .codex/
│   └── PRE_TICKET_WORKFLOW.md
├── .agents/
│   └── skills/
│       ├── graphify-repository-analysis/
│       │   └── SKILL.md
│       └── code-review-graph-analysis/
│           └── SKILL.md
└── .gitignore.recommended
```

`AGENTS.md`, `.codex/`, and `.agents/skills/` should be committed.

`.gitignore.recommended` is a reviewable fragment. Merge its relevant rules into
the repository's existing `.gitignore`; do not blindly replace the existing file.

The package deliberately does not include wrapper scripts because the exact
installed Graphify and CRG CLI syntax must first be confirmed in the project
environment. The workflow now prohibits Codex from inventing those commands.
