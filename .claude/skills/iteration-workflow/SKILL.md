---
name: iteration-workflow
description: Use for every development task/iteration in this project (planning, analysis, writing code, testing, committing) — enforces the strict Plan → Analysis → Code → Tests → Commit workflow with mandatory user approval between steps and per-file explanations. Triggers on any request to implement, add, fix, or change code in this repo.
---

# Working Method — Iteration Workflow

## How We Work Together
Every task (iteration) follows this strict process. Do NOT skip steps.

---

## Step 1: PLAN (before any code)
When given an iteration task:
1. Read the task carefully
2. Write a **plan in the chat** that includes:
   - What needs to be built
   - Which files will be created or modified
   - What classes and functions are needed
   - Edge cases and potential problems
   - How this connects to existing code

**Wait for user approval before continuing.**

---

## Step 2: ANALYSIS
After plan is approved:
1. Identify all **edge cases**
2. Define **test cases** that should be written
3. Explain any design decisions and **why** this approach was chosen over alternatives
4. If touching multiple files — explain how they interact

**Wait for user approval before continuing.**

---

## Step 3: WRITE CODE
After analysis is approved:
1. Write the code
2. For **every class**: explain what it is responsible for and why it was designed this way
3. For **every function**: explain what it does and why
4. If multiple files are involved: explain how they work together
5. Follow all rules from CLAUDE.md (DRY, SRP, Encapsulation, no hard-coding)

**Split large tasks into sub-steps. After each sub-step — wait for user approval.**

**After every file (or sub-step) you write or change, before moving to the next one, explain:**
- Exactly what you did in that file
- Which functions/classes you added or changed
- The shape/design of the solution (how the pieces fit together)
- Which part of the task/requirement this specific file or step solves

---

## Step 4: TESTS
After code is written:
1. Write unit tests for everything added
2. Aim for 100% coverage
3. Explain what each test is checking and why

---

## Step 5: COMMIT
After tests pass:
1. Suggest a commit message
2. Wait for approval
3. Commit

---

## Important Rules
- NEVER write code before the plan is approved
- NEVER skip the explanation of classes and functions
- ALWAYS split large tasks into smaller approved steps
- ALWAYS check: DRY, SRP, Encapsulation, no magic numbers
