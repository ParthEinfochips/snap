echo "#######################################"
echo "#            Python_Libs               "
echo "#######################################"

mkdir -p /usr/local/lib/python2.7
mkdir -p /usr/local/lib/python2.7/dist-packages
cp -rf dist-packages/* /usr/local/lib/python2.7/dist-packages

echo "INFO : Copying site-packages"
mkdir -p /usr/lib/python2.7
cp -rf dist-packages/* /usr/lib/python2.7/site-packages

cd -
export LD_LIBRARY_PATH=/opt/AlljoynLib/
