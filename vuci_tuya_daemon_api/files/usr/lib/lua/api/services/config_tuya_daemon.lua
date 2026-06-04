local ConfigService = require("api/ConfigService")

local Tuya_daemon = ConfigService:new({
	-- delete = false,          -- Disable deletion of UCI sections
	-- create = false,          -- Disable creation of UCI sections
	-- general_section = "main",-- General UCI section name
	-- anonymous = true,        -- Create UCI anonymous sections
	-- increment_name = true,   -- Create UCI sections with numeric incremental names
})

local ConfigTuya_daemon = Tuya_daemon:section(
	"tuya_daemon", -- UCI config name
	"tuya_daemon"  -- UCI section type
)
ConfigTuya_daemon:make_primary()
ConfigTuya_daemon.default_options.id.maxlength = 8 -- Default id option can also have validations
-- ConfigTuya_daemon.order_by = "option" -- Order UCI config by provided option
-- ConfigTuya_daemon.sort_response_by = "option" -- Order API response by provided option

function ConfigTuya_daemon:create_defaults(sid)
	-- Default values to be added with every creation
	return {
		productID = "",
		deviceID = "",
		deviceSecret = "",
		enabled = true,
	}
end

	local opt_text = ConfigTuya_daemon:option("text")
		-- opt_text.cfg_require = true -- Option is required
		opt_text.maxlength = 100
		opt_text.minlength = 5

	local opt_product_id = ConfigTuya_daemon:option("product_id")
		opt_product_id.maxlength = 128

	local opt_device_id = ConfigTuya_daemon:option("device_id")
		opt_device_id.maxlength = 128

	local opt_device_secret = ConfigTuya_daemon:option("device_secret")
		opt_device_secret.maxlength = 128

	local opt_enabled = ConfigTuya_daemon:option("enabled")
		function opt_enabled:validate(value)
			return self.dt:is_bool(value)
		end

	local opt_select = ConfigTuya_daemon:option("select")
		function opt_select:validate(value)
			return self.dt:check_array(value, { "first", "second" })
		end

	local opt_multi_select = ConfigTuya_daemon:option("multi_select", { list = true })
		opt_multi_select.maxlength = 64

	local opt_test = ConfigTuya_daemon:option("test")
		opt_test.readonly = true

return ConfigTuya_daemon