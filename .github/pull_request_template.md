## Summary

<!-- Required: Briefly describe what this PR does and why. -->

## Changes

<!-- Required: List the notable code, docs, test, or configuration changes. -->

## Testing

<!-- Required: Describe what you ran locally and the results. Include relevant commands. Verifying the build locally is required. -->

Diagnostic bundle:

- Commit the real `diagnostic/build-<commit>.json` and matching encrypted `.logd`
  artifact(s) generated after your code changes.
- Do not submit `diagnostic/build-00000000.json` or
  `diagnostic/build-00000000.logd`; those checked-in files are stubs and do not
  satisfy PR validation.

## Checklist

- [ ] Relevant modules affected by these changes build locally
- [ ] Tests pass locally
- [ ] Real diagnostic build log and matching metadata are committed in this PR
- [ ] Diagnostic artifacts are named for a real commit, not `build-00000000`
- [ ] Documentation has been updated, if applicable
- [ ] Configuration or schema changes are documented, if applicable
- [ ] No generated build artifacts are committed, except the required diagnostic build log
- [ ] Changes are scoped to the PR purpose and avoid unrelated cleanup
- [ ] Security, privacy, and error-handling implications have been considered

---

- [ ] I would like to request that my diagnostic build log is removed before merging
