---
name: grill-me
description: Interview the owner relentlessly about a requirement, plan or design until every ambiguity is resolved. Use before writing a ticket, before a Level 2 plan, or when the owner says "grill me" / "погоняй меня".
---

# Grill me

Goal: reach a shared, unambiguous understanding before any code is written.
Cheap questions now beat an expensive review loop later.

## How

1. Restate the request in 2–3 sentences in your own words.
2. Read just enough of the code (CRG `get_minimal_context_tool`, `Docs/ARCHITECTURE.md`,
   related `Tickets/`) to ask *informed* questions. Never ask what the code already answers.
3. Ask questions **one branch at a time**, at most 3–4 per round, each with your
   recommended answer and why. Prefer multiple-choice (AskUserQuestion) when the
   options are clear.
4. Walk the decision tree depth-first. Cover, as relevant:
   - goal and the user-visible outcome; who uses it and when;
   - scope and explicit non-goals;
   - UI: where it lives in the panel, labels, enable/disable states, defaults, units;
   - data: coordinate spaces, units (cm/m), precision, limits, invalid input;
   - Unreal specifics: editor-only vs runtime, Undo/Redo, dirtying, World Partition,
     level change, missing Landscape, GC/lifetime, threading;
   - performance limits (instance counts, heightmap sizes);
   - interaction with existing features (what must not change);
   - acceptance criteria that can be checked, automation tests, manual checks;
   - whether owner visual verification is required before merge.
5. Push back. If the owner's idea has a cheaper or safer alternative, say so with
   arguments. Do not just agree.
6. Stop when every acceptance criterion is testable and no open question changes
   the implementation. Summarise decisions as a bullet list and ask for a final OK.
