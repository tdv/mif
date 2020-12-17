mkdir third_party_sources ||:
cd third_party_sources
git clone --branch v1.2.9 https://github.com/madler/zlib.git

wget https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz
tar zxvf boost_1_75_0.tar.gz
mv boost_1_75_0 boost
rm boost_1_75_0.tar.gz

wget https://github.com/postgres/postgres/archive/REL9_6_2.tar.gz
tar zxvf REL9_6_2.tar.gz
mv postgres-REL9_6_2 libpq
rm REL9_6_2.tar.gz

wget http://github.com/zeux/pugixml/releases/download/v1.8/pugixml-1.8.tar.gz
tar zxvf pugixml-1.8.tar.gz
mv pugixml-1.8 pugixml
rm pugixml-1.8.tar.gz
