package logging

import (
	"log/slog"
	"net/http"
	"os"
)

func ExampleLogger_httpError() {
	handler := slog.NewJSONHandler(os.Stdout, nil)

	logger := slog.NewLogLogger(handler, slog.LevelError)

	_ = http.Server{
		// this API only accepts `log.Logger`
		ErrorLog: logger,
	}
}
