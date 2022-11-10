from movesrwth/storm:latest

RUN apt-get update && apt-get install -y build-essential libboost-all-dev git cmake clang 

ENV STORM_DIR=/opt/storm
ENV STAMINA_HOME=/opt/stamina-storm/build

COPY . $STAMINA_HOME/..
RUN mkdir -p $STAMINA_HOME
WORKDIR $STAMINA_HOME

RUN cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DSTORM_PATH=$STORM_DIR
RUN make -j$(nproc --all)

