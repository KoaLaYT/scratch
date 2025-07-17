package logging_test

import (
	"context"
	"log/slog"
	"os"
	"time"

	"github.com/koalayt/scratch/go/logging"
)

func replaceTime(groups []string, a slog.Attr) slog.Attr {
	if a.Key == "time" {
		a.Value = slog.TimeValue(time.Unix(0, 0))
	}
	return a
}

func ExampleLogger_contextHandler() {
	h := logging.ContextHandler{
		slog.NewJSONHandler(os.Stdout, &slog.HandlerOptions{
			ReplaceAttr: replaceTime, // to make time not change
		}),
	}
	logger := slog.New(h)

	ctx := logging.AppendCtx(context.Background(), slog.String("trace_id", "ergji"))

	logger.InfoContext(ctx, "hello")

	// Output:
	// {"time":"1970-01-01T08:00:00+08:00","level":"INFO","msg":"hello","trace_id":"ergji"}
}
