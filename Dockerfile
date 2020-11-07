FROM gcc:10.2 as build

COPY . .
RUN make main

FROM debian:buster-slim
COPY --from=build main main
CMD ["./main"]
