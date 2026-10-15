# Automation setup

Target pipeline:

```text
push to feature branch / PR ──► CI (self-hosted Windows + UE 5.7: build + tests)
                                   │ success
                                   ▼
                           Claude review (GitHub-hosted Ubuntu, Opus)
                                   │
                    Reviews/review-<ID>.md + PR comment
```

Both workflows are **off by default** and are switched on with repository
variables, so merging this setup changes nothing until you are ready.

## 0. Local developer machine (once)

1. Install Claude Code (desktop app with the *Code* tab, or CLI) and log in with
   the same account that has the YouTrack connector.
2. Install code-review-graph and build the index from the plugin root:
   ```powershell
   pip install code-review-graph      # or: uv tool install code-review-graph
   code-review-graph build
   ```
   The MCP server is declared in `.mcp.json` (`uvx code-review-graph serve`),
   so `uv` must be installed (`pip install uv` or https://docs.astral.sh/uv/).
3. Environment variables (new terminal afterwards):
   ```powershell
   setx LHT_UE_ROOT "C:\Program Files\Epic Games\UE_5.7"
   setx LHT_HOST_UPROJECT "J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject"
   setx CONTEXT7_API_KEY "<your Context7 key>"
   ```
4. Open the plugin folder in Claude Code, approve the project MCP servers, run
   `/mcp` and check `code-review-graph`, `context7` and YouTrack are connected.
5. Smoke test: `powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Invoke-Validation.ps1`.

## 1. Make the repository private (recommended)

Settings → General → Danger zone → Change visibility → Private.
Self-hosted runners must not serve public repositories: anyone could open a PR
and run code on your machine. `ci.yml` additionally refuses fork PRs.

## 2. Self-hosted runner (CI)

1. Settings → Actions → Runners → *New self-hosted runner* → Windows x64.
   Follow the commands; when asked for labels add `ue57`.
   Install it as a service (`./config.cmd --runasservice`) or run `run.cmd` manually.
2. Create a **separate** host project for CI (copy of `UE57Editor` without the
   plugin, e.g. `J:\CI\UE57Host\UE57Host.uproject`). CI mirrors the plugin into its
   `Plugins\` folder (`robocopy /MIR`), so never point it at your working project.
3. Settings → Secrets and variables → Actions → *Variables*:
   | Variable | Value |
   |---|---|
   | `LHT_SELF_HOSTED_CI` | `true` |
   | `LHT_UE_ROOT` | `C:\Program Files\Epic Games\UE_5.7` |
   | `LHT_CI_HOST_UPROJECT` | `J:\CI\UE57Host\UE57Host.uproject` |
4. Open any PR → the *CI* workflow should build and run tests; logs are attached
   as the `validation-<run id>` artifact.

Notes: the runner machine must be on, and the Unreal Editor must not hold the CI
host project open. Build + tests take roughly 5–20 minutes.

## 3. Automatic review

1. Install the Claude GitHub App on the repository:
   https://github.com/apps/claude/installations/select_target
2. Create an auth token for the workflow — either
   - subscription: run `claude setup-token` locally and store the value as secret
     `CLAUDE_CODE_OAUTH_TOKEN`; or
   - API billing: store an API key as secret `ANTHROPIC_API_KEY` and switch the
     commented line in `claude-review.yml`.
3. Variable `LHT_AUTO_REVIEW` = `true`.
4. After a green CI run on a PR, *Claude review* starts, writes
   `Reviews/review-<ID>.md` (commit marked `[skip ci]`), and comments on the PR.

Caveats:
- Do not enable "Require status checks" branch protection on `main` for now: the
  reviewer's `[skip ci]` commit has no CI run and would block the merge button.
- Review runs on Opus with `--max-turns 80`; each review consumes your plan
  quota / API budget. Re-pushes cancel an in-progress review for the same branch.
- YouTrack is not reachable from GitHub Actions; the CI reviewer skips YouTrack
  updates. Run `/review <ID>` locally when you want YouTrack updated, or set the
  state yourself.
- The `claude-code-action` + `workflow_run` combination should be verified on the
  first PR; if the action rejects the event, switch to the fallback below.

## 4. Fallback without CI

Review locally in a fresh Claude Code session: `claude "/review UE5-21"`.
