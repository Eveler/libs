<body>
	<parameters>
		<parameter name="$P_2$">
			<parameter_type name="Repeater"/>
			<parameter_data_source value="$P_1$"/>
		</parameter>
	</parameters>
	<keys>
		<key name="$K_4$">
			<key_type name="SQL"/>
			<key_data_type name="DateTime"/>
			<key_data_source value="SELECT now()"/>
		</key>
		<key name="$K_5$">
			<key_type name="Attachment"/>
			<attachment name="Cell_1/Cell_1.mrc"/>
		</key>
	</keys>
</body>