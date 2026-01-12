__kernel void searchComplex(__global int* resultFlag,
                            __global int* outSchedule,
                            __global int* flatOwnerIds,
                            __global int* courseTeachers,
                            __global int* courseGroups,
                            int totalSessions,
                            int totalSlots,
                            int slotsPerDay,
                            ulong totalPerms)
{
    ulong gid = get_global_id(0);

    if (gid >= totalPerms) return;
    if (*resultFlag == 1) return;

    int schedule[32];
    ulong temp = gid;

    for(int i = 0; i < totalSessions; ++i) {
        schedule[i] = temp % totalSlots;
        temp /= totalSlots;
    }

    int valid = 1;
    for(int i = 0; i < totalSessions; ++i) {
        int dayI = schedule[i] / slotsPerDay;
        int courseI = flatOwnerIds[i];

        for(int j = i + 1; j < totalSessions; ++j) {
            int dayJ = schedule[j] / slotsPerDay;
            int courseJ = flatOwnerIds[j];

            if (courseI == courseJ && dayI == dayJ) {
                valid = 0; break;
            }

            if (schedule[i] == schedule[j]) {
                if (courseTeachers[courseI] == courseTeachers[courseJ]) {
                    valid = 0; break;
                }
                if (courseGroups[courseI] == courseGroups[courseJ]) {
                    valid = 0; break;
                }
            }
        }
        if (valid == 0) break;
    }

    if(valid == 1) {
        int old = atomic_cmpxchg(resultFlag, 0, 1);
        if (old == 0) {
            for(int k = 0; k < totalSessions; ++k) {
                outSchedule[k] = schedule[k];
            }
        }
    }
}
