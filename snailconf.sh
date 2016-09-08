#/bin/sh
veryes="no"
force="no"
argument=""
function parseArgument() {
for s in $@
do
	if [ "$s" == "-y" ]; then
		veryes="yes"
	elif [ "$s" == "-f" ]; then
		force="yes"
	elif [ "$s" == "--force" ]; then
		force="yes"
	elif [ "$s" == "clean" ]; then
		make distclean
		rm -rf _install
		rm -rf _install_temp
		rm -f inc/snail_version.h
		exit 0
	else
		argument+=$s
		argument+=" "
	fi
done
}
function writeverion() {
	echo "#/bin/sh" > script/version.def
	echo "major=$1" >> script/version.def
	echo "minor=$2" >> script/version.def
	echo "patch=$3" >> script/version.def
	echo "alpha=$4" >> script/version.def
	echo "model=$5" >> script/version.def

}

function readversion() {
read -p "Please enter major number (1-9): " major
read -p "Please enter minor number (0-9): " minor
read -p "Please enter patch number (0-99): " patch
read -p "Please enter alpha word (a-z): " alpha
read -p "Please enter model name: " model
}

function checkversion() {
check=$1
while [ true ]
do
	if [ "$veryes" == "yes" ]; then
		yn="Y"
	else
		read -p "Is your version($check): [Y/n]" yn
		if [ -z $yn ]; then
			yn="Y"
		fi
	fi
	case $yn in
   		[Yy]* ) return;;
   		[Nn]* ) readversion;version=`printf "%d.%d.%02d.%04d%s" $major $minor $patch $revision $alpha 2>/dev/null`;check=$version;break;;
       		* ) echo "Please answer yes or no.";;
	esac
done
}


parseArgument $@
mkdir -p output

revision=`svnversion`
if [ -f "script/version.def" ];then
	source script/version.def
else
	readversion
fi

re='^[0-9]+$'
if [ "$force" == "no" ]; then
	if ! [[ $revision =~ $re ]] ; then
   		echo -e "It's not a clean repository, please use a commited version and don't modify it.\n(You can use \"-f\" or \"--force\" to ignore it.)" >&2; exit 1
	fi
fi
version=`printf "%d.%d.%02d.%04d%s" $major $minor $patch $revision $alpha 2>/dev/null`
release=`printf "%d.%d.%02d" $major $minor $patch`

checkversion $version
writeverion "$major" "$minor" "$patch" "$alpha" "$model"

echo "#define SNAIL_RELEASE_VERSION \"SNAIL.$release\"" > inc/snail_version.h
echo "#define SNAIL_FACTORY_VERSION \"SNAIL.$version\"" >> inc/snail_version.h
echo "#define SNAIL_MODEL \"SNAIL.$model\"" >> inc/snail_version.h
echo $argument
./configure $CONF_FLAGS --prefix=/usr $argument
#make
if [ -f "/usr/bin/perl" ];then
	make 2>&1 | /usr/bin/perl -wln -M'Term::ANSIColor' -e '
	m/Error/i and print "\e[1;91m", "$_", "\e[0m"
	or
	m/Warning/i and print "\e[1;93m", "$_", "\e[0m"
	or
	print; '
else
        make
fi
mkdir -p _install
make install DESTDIR=`pwd`/_install
mkdir -p _install_temp
cp -a _install/* _install_temp/.
cd _install_temp/usr
find * -mindepth 1 -type d > ../filelist
find * -type f >> ../filelist
find * -type l >> ../filelist
cd ../..
cp script/install.sh _install_temp/.
#makeself --gzip ./_install_temp ./sennet-$model-$version.run "ADVANTECH WISE IOT GATEWAY for $model" ./install.sh
rm -rf _install_temp
