import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

basic_espnow_ns = cg.esphome_ns.namespace("basic_espnow")
BasicESPNow = basic_espnow_ns.class_("BasicESPNow", cg.Component)

CONF_PEERS = "peers"

# Accept a list of MAC address strings
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(BasicESPNow),
    cv.Optional(CONF_PEERS, default=[]): cv.ensure_list(cv.mac_address),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    for mac in config[CONF_PEERS]:
        cg.add(var.add_peer(mac.parts))
    return var