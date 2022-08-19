from movesrwth/storm:1.7.0

RUN apt-get update && apt-get install -y build-essential libboost-all-dev git cmake clang 

ENV STORM_DIR=/opt/storm
ENV STAMINA_HOME=/opt/stamina-storm/build

RUN git clone https://github.com/fluentverification/stamina-cplusplus /opt/stamina-storm
RUN mkdir $STAMINA_HOME
RUN cd $STAMINA_HOME && cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DSTORM_PATH=$STORM_DIR
RUN cd $STAMINA_HOME && make -j$(nproc --all)