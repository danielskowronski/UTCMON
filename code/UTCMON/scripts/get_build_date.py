Import("env")
from datetime import datetime, timezone
build_date = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")

env.Append(
    BUILD_FLAGS=[f"-D BUILD_DATE=\\\"{build_date}\\\""]
)
