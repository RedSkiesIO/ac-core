#ifndef USGOV_be2f10446a35e971a3b843dc538fa9e3a045ee35d93437ab1efadd3069618800
#define USGOV_be2f10446a35e971a3b843dc538fa9e3a045ee35d93437ab1efadd3069618800

#include <stdint.h>
#include <gov/peer/protocol.h>

namespace usgov {
namespace protocol {

	static constexpr uint16_t nova_base{800};
	static constexpr uint16_t nova_evidence{((nova_base+1)<<2)+evidence_suffix};
//	static constexpr uint16_t nova_reading{((nova_base+2)<<2)+evidence_suffix};
//	static constexpr uint16_t nova_unload{((nova_base+3)<<2)+evidence_suffix};
	static constexpr uint16_t nova_compartiment_query{((nova_base+4)<<2)+query_suffix};
	static constexpr uint16_t nova_response{((nova_base+5)<<2)+query_suffix};
}
}

#endif
