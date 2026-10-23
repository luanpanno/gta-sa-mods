#include "powers/AddMoney.h"

#include "plugin.h"
#include "CPlayerInfo.h"
#include "CPlayerPed.h"

#include <algorithm>

namespace {

const int kAmounts[] = {1000, 10000, 100000, 1000000, 10000000};
const char* kLabels[] = {"$1,000", "$10,000", "$100,000", "$1,000,000", "$10,000,000"};
const int kCount = sizeof(kAmounts) / sizeof(kAmounts[0]);
const int kMaxMoney = 999999999; // the HUD shows 9 digits

} // namespace

const char* AddMoney::PickerValue() const {
    return kLabels[m_index];
}

void AddMoney::PickerStep(int delta) {
    m_index = std::clamp(m_index + (delta > 0 ? 1 : -1), 0, kCount - 1); // no wrap: amounts go up/down
}

void AddMoney::OnActivate(CPlayerPed* player) {
    CPlayerInfo* info = player->GetPlayerInfoForThisPlayerPed();
    if (!info)
        return;
    long long money = (long long)info->m_nMoney + kAmounts[m_index];
    info->m_nMoney = (int)std::min<long long>(money, kMaxMoney);
}
