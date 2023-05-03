from movesrwth/storm:1.7.1

RUN apt-get update && apt-get install -y build-essential libboost-all-dev git cmake clang 
RUN apt-get install git

ENV STORM_DIR=/opt/storm
ENV STAMINA_HOME=/opt/stamina-storm/build
ENV STAMINA_DIR=/opt/stamina-storm

RUN git clone https://github.com/fluentverification/stamina-storm $STAMINA_DIR
RUN mkdir -p $STAMINA_HOME

COPY . $STAMINA_DIR
WORKDIR $STAMINA_HOME
RUN cmake .. -DSTORM_DIR=$STORM_DIR
RUN make -j$(nproc --ignore=1)
RUN export PATH=$PATH:/opt/stamina-storm/build >> ~/.shinit

