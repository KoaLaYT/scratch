package logging

import (
	"context"
	"log/slog"
	"os"
	"path/filepath"
	"time"

	"github.com/mdobak/go-xerrors"
)

type stackFrame struct {
	Func   string `json:"func"`
	Source string `json:"source"`
	Line   int    `json:"line"`
}

func replaceAttr(_ []string, a slog.Attr) slog.Attr {
	if a.Key == "time" {
		a.Value = slog.TimeValue(time.Unix(0, 0))
		return a
	}

	switch a.Value.Kind() {
	case slog.KindAny:
		switch v := a.Value.Any().(type) {
		case error:
			a.Value = fmtErr(v)
		}
	}

	return a
}

// marshalStack extracts stack frames from the error
func marshalStack(err error) []stackFrame {
	trace := xerrors.StackTrace(err)

	if len(trace) == 0 {
		return nil
	}

	frames := trace.Frames()

	s := make([]stackFrame, len(frames))

	for i, v := range frames {
		f := stackFrame{
			Source: filepath.Join(
				filepath.Base(filepath.Dir(v.File)),
				filepath.Base(v.File),
			),
			Func: filepath.Base(v.Function),
			Line: v.Line,
		}

		s[i] = f
	}

	return s
}

// fmtErr returns a slog.Value with keys `msg` and `trace`. If the error
// does not implement interface { StackTrace() errors.StackTrace }, the `trace`
// key is omitted.
func fmtErr(err error) slog.Value {
	var groupValues []slog.Attr

	groupValues = append(groupValues, slog.String("msg", err.Error()))

	frames := marshalStack(err)

	if frames != nil {
		groupValues = append(groupValues,
			slog.Any("trace", frames),
		)
	}

	return slog.GroupValue(groupValues...)
}

func ExampleLogger_errorStackFrames() {
	h := slog.NewJSONHandler(os.Stdout, &slog.HandlerOptions{
		ReplaceAttr: replaceAttr,
	})

	logger := slog.New(h)

	ctx := context.Background()

	err := xerrors.New("something happened")

	logger.ErrorContext(ctx, "image uploaded", slog.Any("error", err))

	// Output:
	// {"time":"1970-01-01T08:00:00+08:00","level":"ERROR","msg":"image uploaded","error":{"msg":"something happened","trace":[{"func":"logging.ExampleLogger_errorStackFrames","source":"logging/example_error_stackframes_test.go","line":92},{"func":"testing.runExample","source":"testing/run_example.go","line":63},{"func":"testing.runExamples","source":"testing/example.go","line":41},{"func":"testing.(*M).Run","source":"testing/testing.go","line":2144},{"func":"main.main","source":"_testmain.go","line":49},{"func":"runtime.main","source":"runtime/proc.go","line":283},{"func":"runtime.goexit","source":"runtime/asm_arm64.s","line":1223}]}}
}
