mkdir third_party_sources ||:
cd third_party_sources
git clone --branch v1.2.8 https://github.com/madler/zlib.git
git clone --branch 1.8.0 https://github.com/open-source-parsers/jsoncpp.git
git clone --branch release-2.1.8-stable https://github.com/libevent/libevent.git

wget https://sourceforge.net/projects/boost/files/boost/1.65.1/boost_1_65_1.tar.gz
tar zxvf boost_1_65_1.tar.gz
mv boost_1_65_1 boost
rm boost_1_65_1.tar.gz

wget https://github.com/postgres/postgres/archive/REL9_6_2.tar.gz
tar zxvf REL9_6_2.tar.gz
mv postgres-REL9_6_2 libpq
rm REL9_6_2.tar.gz

wget https://www.sqlite.org/src/tarball/sqlite.tar.gz
tar zxvf sqlite.tar.gz
rm sqlite.tar.gz

wget http://github.com/zeux/pugixml/releases/download/v1.8/pugixml-1.8.tar.gz
tar zxvf pugixml-1.8.tar.gz
mv pugixml-1.8 pugixml
rm pugixml-1.8.tar.gz
