#pragma once

struct sapp_event;

void appInitialize();
void appUpdate();
void appShutdown();

void appNotifyEvent(const sapp_event* );

void * appGetWindowHdl();
