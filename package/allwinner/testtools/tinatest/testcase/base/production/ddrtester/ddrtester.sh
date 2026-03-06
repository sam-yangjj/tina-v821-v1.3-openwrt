#!/bin/sh

path_in_config_tree="/base/production/ddrtester"
ddr_pll_name=`mjson_fetch $path_in_config_tree/ddr_pll_name`

# 从 clk_summary 中提取 dram 的频率值
frequency=$(cat /sys/kernel/debug/clk/clk_summary | grep ${ddr_pll_name} | awk '{print $5}')

# 检查是否成功获取到值
if [ -z "$frequency" ]; then
  echo "no found dram frequency"
  exit 1
fi

# 定义颜色常量
RED='\033[0;31m'
NC='\033[0m' # No Color

# 打印带红色的结果
frequency=`expr ${frequency} / 2`
echo -e "${RED}current DRAM frequency is: ${frequency} Hz${NC}"
