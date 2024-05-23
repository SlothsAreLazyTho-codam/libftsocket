FROM debian:bullseye

RUN apt-get update -y
RUN apt-get install -y g++ make

WORKDIR /root
COPY . .

RUN make debug

CMD "./a.out"