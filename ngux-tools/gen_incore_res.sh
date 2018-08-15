#!/bin/bash

# 说明 #
# 脚本需要五个参数
# 1. 资源描述目录，其中包含了所有应用程序的资源描述，例如：launcher/include/launcher.res.c，目录结构要保持:应用名字/include/应用名字.res.c，相对于application
# 2. 资源文件目录，其中包含了所有应用程序的资源文件，例如：launcher/res/background.png，目录结构要保持:应用名字/res/<资源路径>，相对于application
# 3. 资源配置名称，来自全局配置，表示一个子目录名字，如mmi_2323
# 4. 生成的资源的输出路径，其中包含了所有应用需要的资源数据，相对于application, 生成如:
# inner-res/src/inner_resource.cpp
# inner-res/launcher/include/...
# inner-res/launcher/src/...
# inner-res/launcher/Makefile
# 5. restool的目录，其中包括了资源工具
################################

# 注意 #
# 1. .res.c声明中，不要出现两次begin_image_res，注释也不行，同理end_image_res也只能出现一次
# 2. .res.c声明中，不要出现声明行内注释可以在声明行上，或者下注释，不要在行尾注释
# 3. 必须保证输入资源描述文件的目录结构，也要保证图片文件的目录结构
# 4. 假设架构如下:
# application/mgapollo
# application/mgapollo/apps/ 应用代码
# application/mgapollo/resdesc/ 应用资源描述，根据全局配置，选择资源子目录
# application/mgapollo/resfile/ 应用资源文件，根据全局配置，选择资源子目录
# application/mgapollo/inner-res/ 由资源转换过来的文件，根据全局配置选择资源子目录
# 5. 声明中，不能有""，如果声明的值为空，需要声明成"null"
################################

####### Global Virables ########
# you may need to change this

if [ x"$3" == x ]
then
global_config_res_name="apollo_def_res_name"
else
global_config_res_name=$3
fi

if [ x"$1" == x ]
then
res_desc_root_dir="mgapollo/resdesc/${global_config_res_name}"
else
res_desc_root_dir=$1
fi

if [ x"$2" == x ]
then
res_file_root_dir="mgapollo/resfile/${global_config_res_name}"
else
res_file_root_dir=$2
fi

if [ x"$4" == x ]
then
output_root_dir="mgapollo/inner-res"
else
output_root_dir=$4
fi

if [ x"$5" == x ]
then
restool_path="$PWD/application/mgapollo/restool"
else
restool_path=$5
fi

# by default , we assume `pwd` is soft
#echo "-------------------- $PATH ----------------- \n"
export PATH="$PATH:$restool_path"
#echo "-------------------- $PATH ----------------- \n"

main_res_cpp_file_name="inner_resouce.cpp"

export res_desc_root_dir res_file_root_dir global_config_res_name output_root_dir



##### End Global Virables ######

######## Functions Lib #########
# logmsg replace echo
function logmsg()
{
    if [ x"${__RES_VERBOSE__}" != x ]
    then
        echo $*
    fi
}

# @param str - str to count
# #return _count_str_return - word in str
function count_str()
{
    export _count_str_return
    _count_str_return=0

    if [ x"$1" == x ]
    then
    _count_str_return=0
    return
    fi

    for item in $1
    do
        let _count_str_return=_count_str_return+1
    done
}
# @param $1 - appname
function parser_res_file_image()
{
    _appname=$1
    res_file="${res_desc_root_dir}/${_appname}/include/${_appname}.res.c"
    image_desc=""
    image_desc_begin_line="-"
    image_desc_end_line="-"
    image_desc_len=0

    image_desc_begin_line=`grep begin_image_res ${res_file} -rn | awk -F: '{print $1}'` 
    image_desc_end_line=`grep end_image_res ${res_file} -rn | awk -F: '{print $1}'` 

    # check begin_image_res mark
    count_str "$image_desc_begin_line"

    if [ 1 -lt $_count_str_return ]
    then
        logmsg -en "Multi >begin_image_res< section Found in file ${res_file}, line ${image_desc_begin_line}\n"
        exit 1
    fi

    # check end_image_res mark
    count_str "$image_desc_end_line"

    if [ 1 -lt $_count_str_return ]
    then
        logmsg -en "Multi >end_image_res< section Found in file ${res_file}, line ${image_desc_end_line}\n"
        exit 1
    fi
    # for now , we get all image desc
    let image_desc_end_line=image_desc_end_line-1
    let image_desc_len=image_desc_end_line-image_desc_begin_line
    image_desc=`head ${res_file} -n ${image_desc_end_line} | tail -n ${image_desc_len}`

    if [ x"${image_desc}" == x ]
    then
        # no image describe
        # return
        logmsg -en "Warning : no image_desc found in ${res_file} !\n"
    fi

    # save to temp file
    # echo -en "${image_desc}" > .__temp_image_desc_file
    __image_desc_key=""
    __image_desc_val=""
    export __image_desc_key __image_desc_val
    # 匹配含有image的行 | 删除空格 | 删除制表符 | 匹配image开头的行 | 剪掉开头的image( | 剪掉结尾的) | 用逗号分割取出key和value，构造赋值语句
    _str_cmd=`echo -en "${image_desc}" | grep "image" | tr -d " " | tr -d "\t" | grep "^image(.*)$" |cut -c7- | tr -d ")" |awk -F, '{print "__image_desc_key=\"$__image_desc_key "$1"\""";__image_desc_val=\"$__image_desc_val "$2"\""}'`

    eval "$_str_cmd"
    # 需要的key和value已经在导出的两个列表里了

    count_str "${__image_desc_key}"
    _len_key=$_count_str_return

    count_str "${__image_desc_val}"
    _len_val=$_count_str_return

    # we can check _len_key and _len_val now, they must be equal !!
    if [ $_len_key -ne $_len_val ]
    then
        logmsg -en " __image_desc_key's len is NOT equal with __imgae_desc_val's len !!! Please check file : ${res_file}!!!\n"
        logmsg -en " __image_desc_key's len is ${_len_key}, __image_desc_val's len is ${_len_val} \n"
        logmsg -en " __image_desc_key is ${__image_desc_key}\n __image_desc_val is ${__image_desc_val} \n"
        exit 1
    fi

    # make string to array
    __image_desc_key=(${__image_desc_key})
    __image_desc_val=(${__image_desc_val})


    # gen main CPP file for this res file
    mkdir -p ${output_root_dir}/${_appname}/include
    mkdir -p ${output_root_dir}/${_appname}/src
    __output_file="${output_root_dir}/${_appname}/include/_img_${_appname}_inner_res.cpp"
    echo -en "" > ${__output_file}
    echo -en "// Main CPP File For Res Desc File \n" >> ${__output_file}
    echo -en "//\tCreated : \n//\ton `uname -a` \n//\tby `whoami` \n//\twhen `date`\n" >> ${__output_file}
    #echo -en "#include \"Apollo.h\"\n\n" >> ${__output_file}
    #echo -en "#include \"NguxCommon.h\"\n" >> ${__output_file}
    #echo -en "#include \"Ngux.h\"\n" >> ${__output_file}
    echo -en "#include \"NGUX.h\"\n" >> ${__output_file}
    echo -en "#include \"apolloconfig.h\"\n" >> ${__output_file}
    echo -en "#ifdef _APOLLO_INNER_RES\n\n" >> ${__output_file}
    echo -en "USE_NGUX_NAMESPACE\n\n" >> ${__output_file}

    # call gen_incore_bitmap to conver image file to CPP file

    __c=0
    __data_list=""
    while [ $__c -lt $_len_key ]
    do
        gen_incore_bitmap "${res_file_root_dir}/${__image_desc_val[${__c}]}" "${output_root_dir}/${_appname}/src/${_appname}_${__image_desc_key[${__c}]}.cpp" "${__image_desc_key[${__c}]}" "${_appname}"
        #__data_list="${__data_list} \t{ \"${__image_desc_val[$__c]}\",\t(NGUByte*)_${_appname}_${__image_desc_key[$__c]}_bitmap,\t1},//${__image_desc_val[${__c}]}\n"
        __data_list="${__data_list} \t{ \"${__image_desc_val[$__c]}\",\t(unsigned char*)_${_appname}_${__image_desc_key[$__c]}_bitmap,\t1},//${__image_desc_val[${__c}]}\n"

        # gen extern declaration
        # echo -en "extern const NGUByte _${_appname}_${__image_desc_key[${__c}]}_bitmap[];\n" >> ${__output_file}
        echo -en "extern const unsigned char _${_appname}_${__image_desc_key[${__c}]}_bitmap[];\n" >> ${__output_file}
        let __c=__c+1
    done
    echo -en "\n\nstatic INNER_RES_INFO _img_${_appname}_inner_res[] = {\n" >> ${__output_file}
    echo -en "${__data_list}" >> ${__output_file}
	echo -en "};\n\n" >> ${__output_file}
    echo -en "#endif   // _APOLLO_INNER_RES\n\n" >> ${__output_file}

    # gen sub makefile
    # makefile head
    cat ._MAKEFILE_TEMPLATE_HEAD > ${output_root_dir}/${_appname}/Makefile
    # makefile localname
    # echo -en "LOCAL_NAME := application/${output_root_dir}/${_appname}\n\n" >> ${output_root_dir}/${_appname}/Makefile
    echo -en "LOCAL_NAME := ${output_root_dir}/${_appname}\n\n" >> ${output_root_dir}/${_appname}/Makefile
    # local module deps
    echo -en "LOCAL_MODULE_DEPENDS :=\n\n" >> ${output_root_dir}/${_appname}/Makefile
    # makefile deps
    echo -en "LOCAL_API_DEPENDS := \\" >> ${output_root_dir}/${_appname}/Makefile
    echo -en "\n" >> ${output_root_dir}/${_appname}/Makefile
    cat ._MAKEFILE_DEPS >> ${output_root_dir}/${_appname}/Makefile
    echo -en "\t\tapplication\n" >> ${output_root_dir}/${_appname}/Makefile
    # makefile tail
    cat ._MAKEFILE_TEMPLATE_TAIL >> ${output_root_dir}/${_appname}/Makefile

}
###### End Functions Lib #######

######## Entry Point ###########
rm -rf ._MAKEFILE_TEMPLATE_HEAD
rm -rf ._MAKEFILE_TEMPLATE_TAIL

# Makefile模板头
cat >._MAKEFILE_TEMPLATE_HEAD <<"_MAKEFILE_TEMPLATE_HEAD"
#**************************************************************#
# COPY THIS FILE AS "Makefile" IN THE "src" DIR OF YOUR MODULE #
# AND CUSTOMIZE IT TO FIT YOUR NEEDS.                          #
#**************************************************************#


## ----------------------------------------------------------- ##
## Don't touch the next line unless you know what you're doing.##
## ----------------------------------------------------------- ##
include ${SOFT_WORKDIR}/env/compilation/mmi_compilevars.mk


## -------------------------------------- ##
## General information about this module. ##
## You must edit these appropriately.     ##
## -------------------------------------- ##


# Name of the module, with toplevel path, e.g. "phy/tests/dishwasher"
_MAKEFILE_TEMPLATE_HEAD

# Makefile模板尾
cat >._MAKEFILE_TEMPLATE_TAIL <<"_MAKEFILE_TEMPLATE_TAIL"
#**************************************************************#
# Set this to a non-null string to signal a toplevel module, like
# phy but not like phy/kitchensink. This defines the behavior of
# make deliv
IS_TOP_LEVEL := yes

# This can be used to define some preprocessor variables to be used in 
# the current module, but also exported to all dependencies.
# This is especially useful in an ENTRY_POINT modules
# Ex. : LOCAL_EXPORT_FLAGS += OS_USED DEBUG will result in 
# -DOS_USED -DDEBUG being passed on each subsequent compile command.
LOCAL_EXPORT_FLAG += __MMI_GB_V5__

## ------------------------------------- ##
##	List all your sources here           ##
## ------------------------------------- ##
# Assembly / C code
S_SRC := ${notdir ${wildcard src/*.S}} # uncomment to take all .S files
C_SRC := ${notdir ${wildcard src/*.c}} # uncomment to take all .c files
CXX_SRC := ${notdir ${wildcard src/*.cpp}} # uncomment to take all .cpp files

## ------------------------------------- ##
##  Do Not touch below this line         ##
## ------------------------------------- ##

include ${SOFT_WORKDIR}/env/compilation/compilerules.mk
_MAKEFILE_TEMPLATE_TAIL

# Makefile依赖列表
cat >._MAKEFILE_DEPS <<"_MAKEFILE_DEPS"
        application/mgapollo/apps \
        application/mgngux \
        application/mgngux/activity \
        application/mgngux/animation \
        application/mgngux/common \
        application/mgngux/commondlg \
        application/mgngux/adapters \
        application/mgngux/devices \
        application/mgngux/drawable \
        application/mgngux/filesystem \
        application/mgngux/graphics \
        application/mgngux/media \
        application/mgngux/mgcl \
        application/mgngux/resource \
        application/mgngux/services \
        application/mgngux/view \
        application/mgminigui/include \
        application/mgminigui/include/ctrl \
        application/mgminigui/src \
        application/mgminigui/src/client \
        application/mgminigui/src/control \
        application/mgminigui/src/control/test \
        application/mgminigui/src/ex_ctrl \
        application/mgminigui/src/font \
        application/mgminigui/src/font/in-core \
        application/mgminigui/src/font/utils \
        application/mgminigui/src/gal \
        application/mgminigui/src/gal/native \
        application/mgminigui/src/gdi \
        application/mgminigui/src/gui \
        application/mgminigui/src/ial \
        application/mgminigui/src/ial/dlcustom \
        application/mgminigui/src/ial/native \
        application/mgminigui/src/ial/netial \
        application/mgminigui/src/ial/nexusial \
        application/mgminigui/src/ial/remoteial \
        application/mgminigui/src/ial/remoteial/client \
        application/mgminigui/src/image \
        application/mgminigui/src/ime \
        application/mgminigui/src/kernel \
        application/mgminigui/src/libc \
        application/mgminigui/src/main \
        application/mgminigui/src/misc \
        application/mgminigui/src/mybmp \
        application/mgminigui/src/newgal \
        application/mgminigui/src/newgal/bf533 \
        application/mgminigui/src/newgal/commlcd \
        application/mgminigui/src/newgal/coolsand \
        application/mgminigui/src/newgal/dfb \
        application/mgminigui/src/newgal/dummy \
        application/mgminigui/src/newgal/em85xxosd \
        application/mgminigui/src/newgal/em85xxyuv \
        application/mgminigui/src/newgal/em86gfx \
        application/mgminigui/src/newgal/fbcon \
        application/mgminigui/src/newgal/fbcon/hi3560aInit \
        application/mgminigui/src/newgal/gdl \
        application/mgminigui/src/newgal/hisi \
        application/mgminigui/src/newgal/mb93493 \
        application/mgminigui/src/newgal/mlshadow \
        application/mgminigui/src/newgal/mstar \
        application/mgminigui/src/newgal/nexus \
        application/mgminigui/src/newgal/pcxvfb \
        application/mgminigui/src/newgal/qvfb \
        application/mgminigui/src/newgal/rtos_xvfb \
        application/mgminigui/src/newgal/s3c6410 \
        application/mgminigui/src/newgal/shadow \
        application/mgminigui/src/newgal/sigma8654 \
        application/mgminigui/src/newgal/stgfb \
        application/mgminigui/src/newgal/stgfb/st_include \
        application/mgminigui/src/newgal/svpxxosd \
        application/mgminigui/src/newgal/utpmc \
        application/mgminigui/src/newgal/wvfb \
        application/mgminigui/src/newgdi \
        application/mgminigui/src/server \
        application/mgminigui/src/standalone \
        application/mgminigui/src/sysres \
        application/mgminigui/src/sysres/bmp \
        application/mgminigui/src/sysres/cursor \
        application/mgminigui/src/sysres/font \
        application/mgminigui/src/sysres/icon \
        application/mgminigui/src/sysres/license \
        application/mgminigui/src/sysres/license/c_files \
        application/mgminigui/src/sysres/license/dat_files \
        application/mgminigui/src/sysres/license/pictures \
        application/mgminigui/src/sysres/license/pictures/common \
        application/mgminigui/src/sysres/license/pictures/hybridos \
        application/mgminigui/src/sysres/license/pictures/mdolphin \
        application/mgminigui/src/sysres/license/pictures/minigui \
        application/mgminigui/src/textedit \
        application/mgmgeff/include \
        application/mgmgeff/src \
        application/mgmgeff/src/effector \
        application/mgmgeff/src/include \
        application/mgmgeff/src/include/effector \
        application/mgentry/main \
        ${MMI_CONFIG} \
        application/systeminc/mmi \
        application/adaptation/communication \
        application/adaptation/custom/app \
        application/adaptation/custom/audio/PSI \
        application/adaptation/custom/common \
        application/adaptation/device \
        application/coolmmi/debug \
        application/systeminc/interface/adaptation \
        application/systeminc/interface/config \
        application/systeminc/interface/hwdrv \
        application/systeminc/interface/l1audio \
        application/systeminc/interface/l1interface \
        application/systeminc/interface/media \
        application/systeminc/interface/mmi \
        application/systeminc/interface/os \
        application/systeminc/interface/ps \
        application/coolmmi/mmi \
        application/coolmmi/mmi/AsyncEvents \
        application/coolmmi/mmi/Audio \
        application/coolmmi/mmi/CallManagement \
        application/coolmmi/mmi/CommonScreens \
        application/coolmmi/mmi/DateTime \
        application/coolmmi/mmi/DebugLevels \
        application/coolmmi/mmi/FileSystem \
        application/adaptation/Framework \
        application/thirdpartylibs/ngux \
        application/thirdpartylibs/gb_code \
        platform \
        platform/csw \
        platform/chip/hal \
        platform/csw/base \
        application/adaptation/device \
        application/coolmmi/media/image \
        application/media/common \
        platform/base/std \
        platform/stack \
        platform/base/sx \
        platform/chip/defs \
        platform/chip/hal \
        platform/csw \
        platform/edrv/pmd \
        platform/svc/uctls \
        platform/svc/umss \
        platform/svc/umss/storage/ram \
        platform/svc/umss/transport/boscsi \
        platform/svc/utraces \
        platform/svc/uvideos \
        platform/csw \
        platform/edrv/pmd \
        platform/edrv/btd/asc3600 \
        platform/mdi \
        application/systeminc/mmi \
        application/systeminc/interface/config \
        application/systeminc/interface/media \
        application/systeminc/interface/mmi \
        application/systeminc/interface/ps \
        application/systeminc/interface/hwdrv \
        application/systeminc/interface/adaptation \
        application/systeminc/interface/l1audio \
        application/systeminc/ps/abm \
        application/systeminc/ps/ems \
        application/systeminc/ps/interfaces \
        application/systeminc/ps/l4 \
        application/systeminc/sst \
        application/mmi_cfg \
        application/coolmmi/mmi/GUI \
        application/coolmmi/mmi \
        application/adaptation/communication \
        application/adaptation/custom/app \
        application/adaptation/custom/audio/PSI \
        application/adaptation/custom/common \
        application/adaptation/custom/ps/PSI \
        application/adaptation/custom/system \
        application/adaptation/debug \
        application/adaptation/device \
        application/adaptation/DI \
        application/adaptation/FileSystem \
        application/adaptation/Framework \
        application/adaptation/java \
        application/adaptation/MiscFramework \
        application/coolmmi/mmi_csdapp/EngineerMode \
_MAKEFILE_DEPS

# ------------- prepare Main Makefile
# ------------- prepare Sub Makefile

# ------------- gen application_list
application_list=""
# search res_desc_root_dir's file or dirs
_list=`find ${res_desc_root_dir}/* -maxdepth 0`
# pick up all dirs
for item in ${_list}
do
    # we just care dir
    if [ -d ${item} ]
    then
        # get base name
        basename="${item##*/}"
        application_list="${application_list} "${basename}
    fi
done
logmsg -en "Res Describe File List : ${application_list} \n"

# -------------- gen incore res main CPP file
mkdir -p ${output_root_dir}/src
__output_file="${output_root_dir}/src/${main_res_cpp_file_name}"
echo -en "" > ${__output_file}
# date & time & machine
echo -en "// Main Res CPP File \n" >> ${__output_file}
echo -en "//\tCreated : \n//\ton `uname -a` \n//\tby `whoami` \n//\twhen `date`\n" >> ${__output_file}
#echo -en "#include \"Apollo.h\"\n\n" >> ${__output_file}
#echo -en "#include \"Ngux.h\"\n" >> ${__output_file}
echo -en "#include \"NGUX.h\"\n" >> ${__output_file}
echo -en "#include \"apolloconfig.h\"\n" >> ${__output_file}
#echo -en "#include \"restypes.h\"\n\n" >> ${__output_file}
echo -en "#ifdef _APOLLO_INNER_RES\n\n" >> ${__output_file}
echo -en "USE_NGUX_NAMESPACE\n\n" >> ${__output_file}

__register_list=""
for app in ${application_list}
do
    if [ ! -e "${res_desc_root_dir}/${app}/include/${app}.res.c" ]
    then
        logmsg -en "${res_desc_root_dir}/${app}/include/${app}.res.c is NOT exist, please check name!\n"
        exit 1
    fi
    __register_list="${__register_list}\n\tRegisterInnerResource(R_TYPE_IMAGE, _img_${app}_inner_res, sizeof(_img_${app}_inner_res)/sizeof(INNER_RES_INFO));"
    echo -en "#include \"_img_${app}_inner_res.cpp\"\n" >> ${__output_file}
done

# function to register inner res
echo -en "\n\nvoid Apollo_register_inner_res(void)\n{" >> ${__output_file}
echo -en "${__register_list}" >> ${__output_file}
echo -en "\n}\n\n" >> ${__output_file}

echo -en "#endif   // _APOLLO_INNER_RES\n\n" >> ${__output_file}

# -------------- walk application_list to gen incore res
for app in ${application_list}
do
    # process image 
    logmsg -en "\nProcess Image Of Res Describe File : ${res_desc_root_dir}/${app}/include/${app}.res.c --------> \n"
    # parser_res_file_image "${res_desc_root_dir}/${app}/include/${app}.res.c"
    parser_res_file_image "${app}"
    logmsg -en "Done <--------- \n"
    # process raw data ...
done

# -------------- gen Main Makefile for inner res
# makefile head
cat ._MAKEFILE_TEMPLATE_HEAD > ${output_root_dir}/Makefile
# makefile localname
# echo -en "LOCAL_NAME := application/${output_root_dir}\n\n" >> ${output_root_dir}/Makefile
echo -en "LOCAL_NAME := ${output_root_dir}\n\n" >> ${output_root_dir}/Makefile
# local module deps
echo -en "LOCAL_MODULE_DEPENDS :=\n" >> ${output_root_dir}/Makefile
for app in ${application_list}
do
    #echo -en "LOCAL_MODULE_DEPENDS += application/${output_root_dir}/${app}\n" >> ${output_root_dir}/Makefile
    echo -en "LOCAL_MODULE_DEPENDS += ${output_root_dir}/${app}\n" >> ${output_root_dir}/Makefile
done
echo -en "\n" >> ${output_root_dir}/Makefile

# makefile deps
echo -en "LOCAL_API_DEPENDS := \\" >> ${output_root_dir}/Makefile
echo -en "\n" >> ${output_root_dir}/Makefile
# inner res deps
for app in ${application_list}
do
    #echo -en "\t\tapplication/${output_root_dir}/${app} "'\\'"\n" >> ${output_root_dir}/Makefile
    echo -en "\t\t${output_root_dir}/${app} "'\\'"\n" >> ${output_root_dir}/Makefile
done
# def deps
cat ._MAKEFILE_DEPS >> ${output_root_dir}/Makefile
echo -en "\t\tapplication\n" >> ${output_root_dir}/Makefile
# makefile tail
cat ._MAKEFILE_TEMPLATE_TAIL >> ${output_root_dir}/Makefile


# clean
rm -rf ._MAKEFILE_TEMPLATE_HEAD
rm -rf ._MAKEFILE_TEMPLATE_TAIL
rm -rf ._MAKEFILE_DEPS
######### End Point ############
