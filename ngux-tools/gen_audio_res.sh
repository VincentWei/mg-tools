#!/bin/bash

# 说明 #
# 目前支持sysres.res.c中声明的audio资源文件以外的新增audio文件，方法是修改文件内容，名称不变
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
output_root_dir="mgapollo/audio-res"
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

# main_res_cpp_file_name="inner_resouce.cpp"

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
function parser_res_file_audio()
{
    _appname=$1
    res_file="${res_desc_root_dir}/${_appname}/include/${_appname}.res.c"
    audio_desc=""
    audio_desc_begin_line="-"
    audio_desc_end_line="-"
    audio_desc_len=0

    audio_desc_begin_line=`grep begin_audio_res ${res_file} -rn | awk -F: '{print $1}'` 
    audio_desc_end_line=`grep end_audio_res ${res_file} -rn | awk -F: '{print $1}'` 

    # check begin_audio_res mark
    count_str "$audio_desc_begin_line"

    if [ 1 -lt $_count_str_return ]
    then
        logmsg -en "Multi >begin_audio_res< section Found in file ${res_file}, line ${audio_desc_begin_line}\n"
        exit 1
    fi

    # check end_audio_res mark
    count_str "$audio_desc_end_line"

    if [ 1 -lt $_count_str_return ]
    then
        logmsg -en "Multi >end_audio_res< section Found in file ${res_file}, line ${audio_desc_end_line}\n"
        exit 1
    fi
    # for now , we get all audio desc
    let audio_desc_end_line=audio_desc_end_line-1
    let audio_desc_len=audio_desc_end_line-audio_desc_begin_line
    audio_desc=`head ${res_file} -n ${audio_desc_end_line} | tail -n ${audio_desc_len}`

    if [ x"${audio_desc}" == x ]
    then
        # no audio describe
        # return
        logmsg -en "Warning : no audio_desc found in ${res_file} !\n"
    fi

    __audio_desc_key=""
    __audio_desc_val=""
    export __audio_desc_key __audio_desc_val
    # 匹配含有audio的行 | 删除空格 | 删除制表符 | 匹配audio开头的行 | 剪掉开头的audio( | 剪掉结尾的) | 用逗号分割取出key和value，构造赋值语句
    _str_cmd=`echo -en "${audio_desc}" | grep "audio" | tr -d " " | tr -d "\t" | grep "^audio(.*)$" |cut -c7- | tr -d ")" |awk -F, '{print "__audio_desc_key=\"$__audio_desc_key "$1"\""";__audio_desc_val=\"$__audio_desc_val "$2"\""}'`

    eval "$_str_cmd"
    # 需要的key和value已经在导出的两个列表里了

    count_str "${__audio_desc_key}"
    _len_key=$_count_str_return

    count_str "${__audio_desc_val}"
    _len_val=$_count_str_return

    # we can check _len_key and _len_val now, they must be equal !!
    if [ $_len_key -ne $_len_val ]
    then
        logmsg -en " __audio_desc_key's len is NOT equal with __imgae_desc_val's len !!! Please check file : ${res_file}!!!\n"
        logmsg -en " __audio_desc_key's len is ${_len_key}, __audio_desc_val's len is ${_len_val} \n"
        logmsg -en " __audio_desc_key is ${__audio_desc_key}\n __audio_desc_val is ${__audio_desc_val} \n"
        exit 1
    fi

    # make string to array
    __audio_desc_key=(${__audio_desc_key})
    __audio_desc_val=(${__audio_desc_val})

    # gen audio data
    __c=0
    while [ $__c -lt $_len_key ]
    do
        # logmsg -en "${res_file_root_dir}/aud_source/${__audio_desc_key[${__c}]}\n"
        # logmsg -en "${output_root_dir}/ringData/include/${__audio_desc_val[${__c}]}\n"

        od -tx4 -An -w4 -v "${res_file_root_dir}/aud_source/${__audio_desc_key[${__c}]}" | awk '{print "0x"$1","}' > ${output_root_dir}/ringData/include/${__audio_desc_val[${__c}]}

        let __c=__c+1
    done
}
###### End Functions Lib #######

parser_res_file_audio "sysres"

######### End Point ############
