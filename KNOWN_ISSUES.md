# Known baseline issues

These are recorded rather than repaired in the initial extraction.

## Historical API generations

`algo.h`, `ops.h`, `next.h`, and `pipeline.h` are successive prototypes with
overlapping behavior. They remain separate at the baseline tag. Development on
`main` treats `next.h` as canonical and moves required composition behavior
there under focused tests.

## Apply composition expectation

The older `algo::apply` invokes each supplied operation with the same input and
output buffers. It does not pipe one operation's output into the next. The
legacy `AlgoApplySpec.MultipleOpsExecuteSequentially` test expects composition
and therefore fails against the current implementation.

## Sink termination differences

The older immediate and operation APIs may continue processing after an output
sink rejects a value. The newer range/sink and pipeline APIs terminate. These
differences require explicit characterization before consolidation.

The canonical `next.h` API now reports `execution_status::truncated` when a
sink rejects a value before range processing completes. Its per-element
`apply_all` operation remains boolean so it can serve as a traversal sink.

## Header self-containment

Some headers currently receive standard-library declarations through
transitive includes. Self-containment will be repaired after the baseline tag.

## Handwritten loops

The imported implementation and legacy tests contain handwritten loops. They
are retained only to establish the working extraction. New code follows the
functional algorithm policy; legacy loops will be replaced incrementally under
behavioral tests.
