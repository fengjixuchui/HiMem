#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/25 下午7:04
# @Author  : zkw
# @File    : Log.py

DEBUG_LOG = True


def i(content: str):
    if DEBUG_LOG:
        print(content)
