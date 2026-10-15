---
name: review
description: Start an independent code review of a ticket's branch or PR using the reviewer agent (Opus, clean context). Use when the owner says "/review UE5-XX", "сделай ревью", "проверь PR".
argument-hint: <ticket ID or PR number>
disable-model-invocation: true
---

# Independent review

Target: `$ARGUMENTS`.

Independence matters more than speed:

1. If this session implemented the ticket, tell the owner the review should run in
   a **fresh** session (`claude "/review <ID>"`) or via CI, and stop — unless the
   owner explicitly insists.
2. Resolve the target: ticket ID → branch `feature/<ID>-*` / `fix/<ID>-*` and its PR
   (`gh pr list --head <branch>`); PR number → ticket ID from the title.
3. Launch the `reviewer` agent with **only** this prompt — no summary of the
   implementation, no opinions:

   > Review ticket `<ID>`, branch `<branch>`, PR `#<n>` (if any), base `main`.
   > Follow your instructions.

4. When the agent returns, relay its verdict and the path of `Reviews/review-<ID>.md`
   to the owner. Do not edit or soften the review.
