#include<epl_plugin.hpp>
#include<sag_connectivity_plugins.hpp>
#include<base64.h>
#include<string>

using com::softwareag::connectivity::AbstractSimpleCodec;
using com::softwareag::connectivity::MapExtractor;
using com::softwareag::connectivity::Message;
using com::softwareag::connectivity::map_t;
using com::softwareag::connectivity::data_t;
using com::softwareag::connectivity::get;
using com::softwareag::connectivity::buffer_t;
using com::softwareag::connectivity::convert_to;
using com::apama::epl::EPLPlugin;

namespace com::apamax {

class Base64Codec: public AbstractSimpleCodec, public EPLPlugin<Base64Codec>
{
public:
	explicit Base64Codec(const CodecConstructorParameters &param)
		: AbstractSimpleCodec(param), base_plugin_t("Base64Plugin")
	{
		MapExtractor configEx(config, "config");
		filterOnTransferEncoding = configEx.get<bool>("filterOnTransferEncoding", false);
		configEx.checkNoItemsRemaining();
	}
	explicit Base64Codec()
		: AbstractSimpleCodec(CodecConstructorParameters("Base64Codec", "", map_t{}, nullptr, nullptr)),
		  base_plugin_t("Base64Plugin")
	{}
	static void initialize(base_plugin_t::method_data_t &md)
	{
		md.registerMethod<data_t(Base64Codec::*)(const char *), &Base64Codec::decode>("decode", "action<string> returns string");
		md.registerMethod<data_t(Base64Codec::*)(const char *), &Base64Codec::encode>("encode", "action<string> returns string");
	}
	data_t decode(const char *str)
	{
		auto res = decode(str, strlen(str));
		return data_t{(const char *)res.begin(), res.size()};
	}
	data_t encode(const char *str)
	{
		auto res = encode(str, strlen(str));
		return data_t{(const char *)res.begin(), res.size()};
	}
	buffer_t decode(const char *str, size_t len)
	{
		buffer_t buf{(size_t)Base64::DecodedLength(str, len)};
		if (!Base64::Decode(str, len, (char*) buf.begin(), Base64::DecodedLength(str, len))) throw std::runtime_error("Decoding Base64 string failed");
		return buf;
	}
	buffer_t encode(const char *str, size_t len)
	{
		buffer_t buf{(size_t)Base64::EncodedLength(len)};
		if (!Base64::Encode(str, len, (char*) buf.begin(), Base64::EncodedLength(len))) throw std::runtime_error("Encoding Base64 string failed");
		return buf;
	}
	virtual bool transformMessageTowardsTransport(Message &m) override
	{
		map_t::iterator it1, it2, it3;
		if (filterOnTransferEncoding && (
				(it1 = m.getMetadataMap().find(httpMeta)) == m.getMetadataMap().end() ||
				(it2 = get<map_t>(it1->second).find(headersMeta)) == get<map_t>(it1->second).end() ||
				(it3 = get<map_t>(it2->second).find(transferTypeMeta)) == get<map_t>(it2->second).end() ||
				it3->second != transferTypeValue)) {
			AbstractSimpleCodec::logger.debug("Skipping message towards transport because transfer encoding is not %s", get<const char*>(transferTypeValue));
			return true;
		}
		auto &payload = get<buffer_t>(m.getPayload());
		m.setPayload(encode((const char *)payload.begin(), payload.size()));
		auto mit = m.getMetadataMap().insert(std::make_pair(httpMeta.copy(), data_t{map_t{}}));
		mit = get<map_t>(mit.first->second).insert(std::make_pair(headersMeta.copy(), data_t{map_t{}}));
		get<map_t>(mit.first->second)[transferTypeMeta] = transferTypeValue.copy();
		return true;
	}
	virtual bool transformMessageTowardsHost(Message &m) override
	{
		map_t::iterator it1, it2, it3;
		if (filterOnTransferEncoding && (
				(it1 = m.getMetadataMap().find(httpMeta)) == m.getMetadataMap().end() ||
				(it2 = get<map_t>(it1->second).find(headersMeta)) == get<map_t>(it1->second).end() ||
				(it3 = get<map_t>(it2->second).find(transferTypeMeta)) == get<map_t>(it2->second).end() ||
				it3->second != transferTypeValue)) {
			AbstractSimpleCodec::logger.debug("Skipping message towards host because transfer encoding is not %s", get<const char*>(transferTypeValue));
			return true;
		}
		auto &payload = get<buffer_t>(m.getPayload());
		m.setPayload(decode((const char *)payload.begin(), payload.size()));
		return true;
	}
private:
	bool filterOnTransferEncoding = false;
	data_t httpMeta{"http"};
	data_t headersMeta{"headers"};
	data_t transferTypeMeta{"content-transfer-encoding"};
	data_t transferTypeValue{"base64"};
};

SAG_DECLARE_CONNECTIVITY_CODEC_CLASS(Base64Codec)
APAMA_DECLARE_EPL_PLUGIN(Base64Codec)

} // com::apamax

